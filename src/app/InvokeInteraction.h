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

#pragma once

#include <core/CHIPCore.h>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeMgr.h>
#include <messaging/Flags.h>
#include <protocols/Protocols.h>
#include <support/BitFlags.h>
#include <support/CodeUtils.h>
#include <support/DLLUtil.h>
#include <support/logging/CHIPLogging.h>
#include <system/SystemPacketBuffer.h>
#include <system/TLVPacketBufferBackingStore.h>

#include <app/MessageDef/CommandDataElement.h>
#include <app/MessageDef/CommandList.h>
#include <app/MessageDef/InvokeCommand.h>
#include "SchemaTypes.h"
#include "basic-types.h"
#include "messaging/ExchangeDelegate.h"
#include "protocols/secure_channel/Constants.h"
#include <functional>

namespace chip {
namespace app {

/**
 * Encapsulates arguments to be passed into SendCommand().
 *
 */
struct CommandParams
{
    enum class TargetType : uint8_t
    {
        kTargetEndpoint     = 0x01, /**< Set when the EndpointId field is valid */
        kTargetGroup        = 0x02, /**< Set when the GroupId field is valid */
    };

    template <class CodeGenType>
    CommandParams(CodeGenType& t, chip::EndpointId endpointId, bool expectsResponses) {
        ClusterId = (uint16_t)t.GetClusterId();
        CommandId = (uint8_t)t.GetCommandId();
        EndpointId = endpointId;
        ExpectsResponses = expectsResponses;
    }

    CommandParams() {
    }

    chip::EndpointId EndpointId = 0;
    chip::GroupId GroupId = 0;
    chip::ClusterId ClusterId = 0;
    chip::CommandId CommandId = 0;
    BitFlags<TargetType> Flags = TargetType::kTargetEndpoint;
    bool ExpectsResponses = false;
};

class InvokeResponder
{
public:
    CHIP_ERROR Init(Messaging::ExchangeContext *apContext, System::PacketBufferHandle &&aBufferHandle);

    enum State
    {
        kStateReady = 0,
        kStateReleased = 1
    };

    class CommandHandler
    {
    public:
        virtual CHIP_ERROR HandleRequest(CommandParams &commandParams, InvokeResponder &invokeInteraction, TLV::TLVReader *payload) = 0;
        virtual ~CommandHandler() {} 
    };

    CHIP_ERROR AddSRequestAndSend(CommandParams aParams, ISerializable *serializable);
    
    template <typename F>
    CHIP_ERROR AddResponse(CommandParams aParams, F f) {
        CHIP_ERROR err = CHIP_NO_ERROR;

        IncrementHoldOffRef();

        err = StartCommandHeader(aParams);
        SuccessOrExit(err);

        // 
        // Invoke the passed in closure that will actually write out the command payload if any
        //
        err = f(*mInvokeCommandBuilder.GetWriter(), TLV::ContextTag(CommandDataElement::kCsTag_Data));
        SuccessOrExit(err);

        mInvokeCommandBuilder.GetCommandListBuilder().GetCommandDataElementBuidler().EndOfCommandDataElement();
        SuccessOrExit((err = mInvokeCommandBuilder.GetCommandListBuilder().GetCommandDataElementBuidler().GetError()));

        DecrementHoldOffRef();

exit:
        return err;
    }

    CHIP_ERROR AddStatusCode(const CommandParams &aParams, const Protocols::SecureChannel::GeneralStatusCode aGeneralCode,
                             const Protocols::Id aProtocolId, const uint16_t aProtocolCode);

    void IncrementHoldOffRef();
    void DecrementHoldOffRef();

private:
    Messaging::ExchangeContext *GetExchangeContext() { return mpExchangeCtx; }
    CHIP_ERROR FinalizeMessage(System::PacketBufferHandle &aBuf);
    CHIP_ERROR SendMessage(System::PacketBufferHandle aBuf);
    friend class InteractionModelEngine;

private:
    int mHoldOffCount = 0;
    State mState;
    friend class TestInvokeInteraction;
    CHIP_ERROR StartCommandHeader(const CommandParams &aParams);
    app::InvokeCommand::Builder mInvokeCommandBuilder;
    chip::System::PacketBufferTLVWriter mWriter;
    Messaging::ExchangeContext *mpExchangeCtx = nullptr;
};
    
class InvokeInitiator : chip::Messaging::ExchangeDelegate
{
public:
    enum State
    {
        kStateReady = 0,           //< The invoke command message has been initialized and is ready
        kStateAwaitingResponse = 1,
    };

    class CommandHandler
    {
    public:
        virtual void HandleDataResponse(CommandParams &commandParams, InvokeInitiator &invokeInteraction, TLV::TLVReader *payload) = 0;
        virtual void HandleStatusResponse(CommandParams &aPath, const Protocols::SecureChannel::GeneralStatusCode aGeneralCode, 
                                          const uint32_t aProtocolId, const uint16_t aProtocolCode) {};
        virtual void HandleError(CHIP_ERROR aError) {};
        virtual ~CommandHandler() {} 
    };

    CHIP_ERROR Init(Messaging::ExchangeManager *apExchangeMgr, CommandHandler *aHandler, NodeId aNodeId, Transport::AdminId aAdminId, SecureSessionHandle * secureSession);

    CHIP_ERROR AddSRequestAndSend(CommandParams aParams, ISerializable *serializable);
    
    template <class F>
    CHIP_ERROR AddRequest(CommandParams aParams, F f) {
        CHIP_ERROR err = CHIP_NO_ERROR;

        //
        // Update our accumulated flag that tracks if any command going into this invoke
        // expects responses
        //
        mExpectsResponses |= aParams.ExpectsResponses;

        err = StartCommandHeader(aParams);
        SuccessOrExit(err);

        // 
        // Invoke the passed in closure that will actually write out the command payload if any
        //
        err = f(*mInvokeCommandBuilder.GetWriter(), TLV::ContextTag(CommandDataElement::kCsTag_Data));
        SuccessOrExit(err);

        mInvokeCommandBuilder.GetCommandListBuilder().GetCommandDataElementBuidler().EndOfCommandDataElement();
        SuccessOrExit((err = mInvokeCommandBuilder.GetCommandListBuilder().GetCommandDataElementBuidler().GetError()));

exit:
        return err;
    }

    template <class F>
    CHIP_ERROR AddRequestAndSend(CommandParams aParams, F f) {
        ReturnErrorOnFailure(AddRequest(aParams, f));
        return Send();
    }

    CHIP_ERROR Send();
    Messaging::ExchangeContext *GetExchange() { return mpExchangeCtx; }
   
private:
    CHIP_ERROR StartCommandHeader(const CommandParams &aParams);
    void CloseExchange();

    void OnMessageReceived(Messaging::ExchangeContext * apExchangeContext, const PacketHeader & aPacketHeader,
                           const PayloadHeader & aPayloadHeader, System::PacketBufferHandle && aPayload) override;
    void OnResponseTimeout(Messaging::ExchangeContext * apExchangeContext) override {}
    

    Messaging::ExchangeContext *GetExchangeContext() { return mpExchangeCtx; }
    friend class InteractionModelEngine;

    CHIP_ERROR FinalizeMessage(System::PacketBufferHandle &aBuf);
    CHIP_ERROR SendMessage(System::PacketBufferHandle aBuf);
  
private: 
    enum Mode
    {
        kModeUnset = 0,
        kModeClientInitiator = 1,
        kModeServerResponder = 2
    };

    friend class TestInvokeInteraction;
    bool mExpectsResponses = false;
    State mState = kStateReady;
    chip::System::PacketBufferTLVWriter mWriter;
    app::InvokeCommand::Builder mInvokeCommandBuilder;
    CommandHandler *mHandler;
    Messaging::ExchangeManager *mpExchangeMgr;
    Messaging::ExchangeContext *mpExchangeCtx = nullptr;
};

} // namespace app
} // namespace chip
