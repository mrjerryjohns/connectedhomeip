/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

/**
 *    @file
 *      This file defines Base class for a CHIP IM Command
 *
 */
#include <platform/CHIPDeviceLayer.h>
#include "InvokeInteraction.h"
#include "InteractionModelEngine.h"
#include "system/SystemPacketBuffer.h"
#include "system/TLVPacketBufferBackingStore.h"
#include <core/CHIPTLVDebug.hpp>

namespace chip {
namespace app {

CHIP_ERROR InvokeInteraction::Init(Messaging::ExchangeContext *apExchangeContext, bool aOnCommandRx)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    //
    // Onus is on the caller to have initialized the exchange correctly before passing
    // it in.
    //
    VerifyOrExit(apExchangeContext != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);

    apExchangeContext->SetResponseTimeout(kImMessageTimeoutMsec);

    mpExchangeCtx = apExchangeContext;

    if (aOnCommandRx) {
        mState = kStateAwaitingResponse;
    }

exit:
    return err;
}

CHIP_ERROR InvokeInteraction::StartCommandHeader(const CommandParams &aParams)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferHandle txBuf;

    VerifyOrExit(mState == kStateReady, err = CHIP_ERROR_INCORRECT_STATE);

    if (mMode == kModeUnset) {
        mMode = kModeClientInitiator;
    }

    //
    // We only allocate the actual TX buffer once we have a command to add, which is now!
    //
    if (!mWriter.HasBuffer()) {
        CommandList::Builder commandListBuilder;

        txBuf = System::PacketBufferHandle::New(1024);
        VerifyOrExit(!txBuf.IsNull(), err = CHIP_ERROR_NO_MEMORY);

        mWriter.Init(std::move(txBuf));

        err = mInvokeCommandBuilder.Init(&mWriter);
        SuccessOrExit(err);

        commandListBuilder = mInvokeCommandBuilder.CreateCommandListBuilder();
        SuccessOrExit(commandListBuilder.GetError());
    }

    {
        CommandDataElement::Builder commandDataElement = mInvokeCommandBuilder.GetCommandListBuilder().CreateCommandDataElementBuilder();
        CommandPath::Builder commandPath = commandDataElement.CreateCommandPathBuilder();

        if (aParams.Flags.Has(CommandParams::TargetType::kTargetEndpoint))
        {
            commandPath.EndpointId(aParams.EndpointId);
        }
        else {
            commandPath.GroupId(aParams.GroupId);
        }

        commandPath.ClusterId(aParams.ClusterId).CommandId(aParams.CommandId).EndOfCommandPath();
        SuccessOrExit((err = commandPath.GetError()));
    }

exit:
    return err;
}


CHIP_ERROR InvokeInteraction::AddStatusCode(const CommandParams &aParams, 
                             const Protocols::SecureChannel::GeneralStatusCode aGeneralCode,
                             const Protocols::Id aProtocolId, const uint16_t aProtocolCode)
{
    CHIP_ERROR err;
    StatusElement::Builder statusBuilder;

    VerifyOrExit(mState == kStateReady, err = CHIP_ERROR_INCORRECT_STATE);
    
    // We don't need to sandwich this with Increment/Decrement calls, since this call to AddStatus
    // can only happen as part of an existing 'HandleMessage' call flow, which in turn is already handling
    // calling those two methods to correctly finalize a message if needed.

    err = StartCommandHeader(aParams);
    SuccessOrExit(err);

    statusBuilder = mInvokeCommandBuilder.GetCommandListBuilder().GetCommandDataElementBuidler().CreateStatusElementBuilder();
    statusBuilder.EncodeStatusElement(aGeneralCode, aProtocolId.ToFullyQualifiedSpecForm(), aProtocolCode).EndOfStatusElement();

    err = statusBuilder.GetError();
    SuccessOrExit(err);

exit:
    return err;
}
 
CHIP_ERROR InvokeInteraction::HandleMessage(System::PacketBufferHandle payload)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    System::PacketBufferTLVReader reader;
    TLV::TLVReader commandListReader;
    InvokeCommand::Parser invokeCommandParser;
    CommandList::Parser commandListParser;

    VerifyOrExit(mState == kStateAwaitingResponse, err = CHIP_ERROR_INCORRECT_STATE);

    // Now that we've received the response, move our state back to ready to permit adding commands
    mState = kStateReady;
    
    if (mMode == kModeUnset) {
        mMode = kModeServerResponder;
    }

    if (mMode == kModeClientInitiator) {
        assert(mState == kStateReady);
    }

    // Reset our expectation of responses, so that it can be set again below if need be.
    mExpectsResponses = false;

    reader.Init(std::move(payload));

    err = reader.Next();
    SuccessOrExit(err);

    err = invokeCommandParser.Init(reader);
    SuccessOrExit(err);

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = invokeCommandParser.CheckSchemaValidity();
    SuccessOrExit(err);
#endif

    err = invokeCommandParser.GetCommandList(&commandListParser);
    SuccessOrExit(err);

    commandListParser.GetReader(&commandListReader);

    while (CHIP_NO_ERROR == (err = commandListReader.Next()))
    {
        CommandParams params;
        CommandDataElement::Parser commandElement;
        CommandPath::Parser commandPath;
        TLV::TLVReader dataReader;
        bool HasData = true;

        VerifyOrExit(chip::TLV::AnonymousTag == commandListReader.GetTag(), err = CHIP_ERROR_INVALID_TLV_TAG);
        VerifyOrExit(chip::TLV::kTLVType_Structure == commandListReader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

        err = commandElement.Init(commandListReader);
        SuccessOrExit(err);

        SuccessOrExit(commandElement.GetCommandPath(&commandPath));
        SuccessOrExit(commandPath.GetClusterId(&params.ClusterId));
        SuccessOrExit(commandPath.GetCommandId(&params.CommandId));
        SuccessOrExit(commandPath.GetEndpointId(&params.EndpointId));

        err = commandElement.GetData(&dataReader);
        if (err == CHIP_END_OF_TLV) {
            HasData = false;
            err = CHIP_NO_ERROR;
        }

        SuccessOrExit(err);

        IncrementHoldoffRef();

        //
        // Look-up the right instance
        //
        if (mMode == kModeServerResponder) {
            bool foundClusterInstance = false;

            InteractionModelEngine::GetInstance()->GetClusterServerSet().ForEachActiveObject([&](ClusterServer **s) {
                if ((*s)->GetClusterId() == params.ClusterId && (*s)->GetEndpoint() == params.EndpointId) {
                    foundClusterInstance = true;

                    err = (*s)->HandleCommand(params, *this, HasData ? &dataReader : NULL);
                    SuccessOrExit(err);

                    return true;
                }

            exit:
                if (err != CHIP_NO_ERROR) {
                    return true;
                }

                return false;
            });

            if (!foundClusterInstance) {
                ChipLogProgress(DataManagement, "Could not find a matching server cluster for command! (ClusterId = %04x, Endpoint = %lu, Command = %lu)", 
                                params.ClusterId, params.EndpointId, params.CommandId);

                err = AddStatusCode(params, Protocols::SecureChannel::GeneralStatusCode::kBadRequest,
                                    Protocols::SecureChannel::Id, Protocols::SecureChannel::kProtocolCodeGeneralFailure);
                SuccessOrExit(err);
            }
        }
        else {
            InteractionModelEngine::GetInstance()->GetClusterClientSet().ForEachActiveObject([&](ClusterClient **c) {
                if ((*c)->GetClusterId() == params.ClusterId && (*c)->GetEndpointId() == params.EndpointId) {
                    err = (*c)->HandleCommand(params, *this, HasData ? &dataReader : NULL);
                    SuccessOrExit(err);

                    return true;
                }

            exit:
                if (err != CHIP_NO_ERROR) {
                    return true;
                }

                return false;
            });
        }

        SuccessOrExit(err);

        mState = kStateReady;

        DecrementHoldoffRef();
    }

    if (err == CHIP_END_OF_TLV) {
        err = CHIP_NO_ERROR;
    }

exit:
   return err; 
}

void InvokeInteraction::IncrementHoldoffRef()
{
    assert(mHoldOffCount >= 0);
    mHoldOffCount++;
}

CHIP_ERROR InvokeInteraction::DecrementHoldoffRef()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(mpExchangeCtx, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mState == kStateReady, err = CHIP_ERROR_INCORRECT_STATE);

    assert(mHoldOffCount > 0);
    mHoldOffCount--;

    VerifyOrExit(mWriter.HasBuffer(), );
    
    if (mHoldOffCount == 0) {
        System::PacketBufferHandle buf;

        err = FinalizeMessage(buf);
        SuccessOrExit(err);

        err = SendMessage(std::move(buf));
        SuccessOrExit(err);

        mState = kStateAwaitingResponse;
    }

    ReleaseOnCompletion();
    
exit:
    return err;
}

void InvokeInteraction::ReleaseOnCompletion()
{
    VerifyOrExit(mState != kStateReleased, );

    if (mHoldOffCount == 0 && mExpectsResponses == false) {
        if (mpExchangeCtx) {
            mpExchangeCtx->Close();
            mpExchangeCtx = nullptr;
        }

        mState = kStateReleased;

        DeviceLayer::PlatformMgr().ScheduleWork(InteractionModelEngine::FreeReleasedInvokeObjects, (intptr_t)InteractionModelEngine::GetInstance());
    }

exit:
    return;
}

// Only get here if we have a valid buffer
CHIP_ERROR InvokeInteraction::FinalizeMessage(System::PacketBufferHandle &aBuf)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(mpExchangeCtx, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mState != kStateAwaitingResponse, err = CHIP_ERROR_INCORRECT_STATE);

    {
        System::PacketBufferHandle txBuf;
        CommandList::Builder commandListBuilder = mInvokeCommandBuilder.GetCommandListBuilder().EndOfCommandList();
        SuccessOrExit(commandListBuilder.GetError());

        mInvokeCommandBuilder.EndOfInvokeCommand();

        err = mInvokeCommandBuilder.GetError();
        SuccessOrExit(err);

        err = mWriter.Finalize(&txBuf);
        SuccessOrExit(err);

        VerifyOrExit(txBuf->EnsureReservedSize(System::PacketBuffer::kDefaultHeaderReserve),
                     err = CHIP_ERROR_BUFFER_TOO_SMALL);

        aBuf = std::move(txBuf);
    }

exit:
    return err;
}

CHIP_ERROR InvokeInteraction::SendMessage(System::PacketBufferHandle aBuf)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(!aBuf.IsNull(), err = CHIP_ERROR_INVALID_ARGUMENT);

    err = mpExchangeCtx->SendMessage(Protocols::InteractionModel::MsgType::InvokeCommandRequest, 
                                     std::move(aBuf), Messaging::SendFlags(Messaging::SendMessageFlags::kExpectResponse));
    SuccessOrExit(err);

exit:
    return err;
}

}
}