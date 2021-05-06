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
 *      This file defines objects for a CHIP Interaction Data model Engine which handle unsolicitied IM message, and
 *      manage different kinds of IM client and handlers.
 *
 */

#include "InteractionModelEngine.h"
#include "Command.h"
#include "CommandHandler.h"
#include "CommandSender.h"
#include "InvokeInteraction.h"
#include <cinttypes>

namespace chip {
namespace app {
InteractionModelEngine sInteractionModelEngine;

InteractionModelEngine::InteractionModelEngine() {}

InteractionModelEngine * InteractionModelEngine::GetInstance()
{
    return &sInteractionModelEngine;
}

CHIP_ERROR InteractionModelEngine::Init(Messaging::ExchangeManager * apExchangeMgr, InteractionModelDelegate * apDelegate)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    mpExchangeMgr = apExchangeMgr;
    mpDelegate    = apDelegate;

    err = mpExchangeMgr->RegisterUnsolicitedMessageHandlerForProtocol(Protocols::InteractionModel::Id, this);
    SuccessOrExit(err);

    mReportingEngine.Init();
    SuccessOrExit(err);

    for (uint32_t index = 0; index < IM_SERVER_MAX_NUM_PATH_GROUPS - 1; index++)
    {
        mClusterInfoPool[index].mpNext = &mClusterInfoPool[index + 1];
    }
    mClusterInfoPool[IM_SERVER_MAX_NUM_PATH_GROUPS - 1].mpNext = nullptr;
    mpNextAvailableClusterInfo                                 = mClusterInfoPool;

exit:
    return err;
}

void InteractionModelEngine::Shutdown()
{
    for (auto & commandSender : mCommandSenderObjs)
    {
        commandSender.Shutdown();
    }

    for (auto & commandHandler : mCommandHandlerObjs)
    {
        commandHandler.Shutdown();
    }

    for (auto & readClient : mReadClients)
    {
        readClient.Shutdown();
    }

    for (auto & readHandler : mReadHandlers)
    {
        readHandler.Shutdown();
    }

    for (uint32_t index = 0; index < IM_SERVER_MAX_NUM_PATH_GROUPS; index++)
    {
        mClusterInfoPool[index].mpNext = nullptr;
        mClusterInfoPool[index].ClearDirty();
    }
    mpNextAvailableClusterInfo = nullptr;
}

CHIP_ERROR InteractionModelEngine::NewCommandSender(CommandSender ** const apCommandSender)
{
    CHIP_ERROR err   = CHIP_ERROR_NO_MEMORY;
    *apCommandSender = nullptr;

    for (auto & commandSender : mCommandSenderObjs)
    {
        if (commandSender.IsFree())
        {
            *apCommandSender = &commandSender;
            err              = commandSender.Init(mpExchangeMgr, mpDelegate);
            if (CHIP_NO_ERROR != err)
            {
                *apCommandSender = nullptr;
                ExitNow();
            }
            break;
        }
    }

exit:
    return err;
}

CHIP_ERROR InteractionModelEngine::NewReadClient(ReadClient ** const apReadClient)
{
    CHIP_ERROR err = CHIP_ERROR_NO_MEMORY;

    for (auto & readClient : mReadClients)
    {
        if (readClient.IsFree())
        {
            *apReadClient = &readClient;
            err           = readClient.Init(mpExchangeMgr, mpDelegate);
            if (CHIP_NO_ERROR != err)
            {
                *apReadClient = nullptr;
            }
            return err;
        }
    }

    return err;
}

InvokeInteraction* InteractionModelEngine::NewInvokeInteraction(ClusterClient *aClient)
{
    InvokeInteraction *pInteraction = nullptr;
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (aClient) {
        mInvokeInteractions.ForEachActiveObject([&](InvokeInteraction *apInteraction) {
            if (apInteraction->GetExchangeContext()->GetSecureSessionHandle().GetAdminId() == aClient->GetRemoteAdmin() &&
                apInteraction->GetExchangeContext()->GetSecureSessionHandle().GetPeerNodeId() == aClient->GetRemoteNodeId()) {
                pInteraction = apInteraction;
                return false;
            }

            return true;
        });
    }

    if (!pInteraction) {
        Messaging::ExchangeContext *ec;

        pInteraction = mInvokeInteractions.CreateObject();
        VerifyOrExit(pInteraction, err = CHIP_ERROR_NO_MEMORY);

        // Allocate an exchange
        ec = mpExchangeMgr->NewContext({aClient->GetRemoteNodeId(), 0, aClient->GetRemoteAdmin()}, this);
        VerifyOrExit(ec != nullptr, err = CHIP_ERROR_NO_MEMORY);

        err = pInteraction->Init(ec);
        SuccessOrExit(err);
    }

exit:
    return pInteraction;
}

CHIP_ERROR InteractionModelEngine::RegisterClient(ClusterClient *apClusterClient)
{
    ClusterClient **client = mClusterClients.CreateObject();
    if (client == nullptr) {
        return CHIP_ERROR_NO_MEMORY;
    }

    *client = apClusterClient;
    return CHIP_NO_ERROR;
}

CHIP_ERROR InteractionModelEngine::RegisterServer(ClusterServer *apClusterServer)
{
    ClusterServer **server = mClusterServers.CreateObject();
    if (server == nullptr) {
        return CHIP_ERROR_NO_MEMORY;
    }

    *server = apClusterServer;
    return CHIP_NO_ERROR;
}

void InteractionModelEngine::OnUnknownMsgType(Messaging::ExchangeContext * apExchangeContext, const PacketHeader & aPacketHeader,
                                              const PayloadHeader & aPayloadHeader, System::PacketBufferHandle aPayload)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    ChipLogDetail(DataManagement, "Msg type %d not supported", aPayloadHeader.GetMessageType());

    // Todo: Add status report
    // err = SendStatusReport(ec, kChipProfile_Common, kStatus_UnsupportedMessage);
    // SuccessOrExit(err);

    apExchangeContext->Close();
    apExchangeContext = nullptr;

    ChipLogFunctError(err);

    // Todo: Fix the below check after the above status report is implemented.
    if (nullptr != apExchangeContext)
    {
        apExchangeContext->Abort();
    }
}

void InteractionModelEngine::OnInvokeCommandRequest(Messaging::ExchangeContext * apExchangeContext,
                                                    const PacketHeader & aPacketHeader, const PayloadHeader & aPayloadHeader,
                                                    System::PacketBufferHandle aPayload)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    InvokeInteraction *interaction = nullptr;
    
    if (mInvokeInteractions.ForEachActiveObject([&](InvokeInteraction *apInteraction) {
        printf("Found one - %x (%x)\n", (uintptr_t)apInteraction->GetExchangeContext(), (uintptr_t)apExchangeContext);

        if (apInteraction->GetExchangeContext() == apExchangeContext) {
            printf("Done\n");
            interaction = apInteraction;
            return false;        
        }

        return true;
    })) {
        interaction = mInvokeInteractions.CreateObject();
        assert(interaction != nullptr);

        err = interaction->Init(apExchangeContext);
        SuccessOrExit(err);
    }

    err = interaction->HandleMessage(std::move(aPayload));
    SuccessOrExit(err);

exit:
    ChipLogFunctError(err);
}

void InteractionModelEngine::OnReadRequest(Messaging::ExchangeContext * apExchangeContext, const PacketHeader & aPacketHeader,
                                           const PayloadHeader & aPayloadHeader, System::PacketBufferHandle aPayload)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    ChipLogDetail(DataManagement, "Receive Read request");

    for (auto & readHandler : mReadHandlers)
    {
        if (readHandler.IsFree())
        {
            err = readHandler.Init(mpDelegate);
            SuccessOrExit(err);
            err = readHandler.OnReadRequest(apExchangeContext, std::move(aPayload));
            SuccessOrExit(err);
            apExchangeContext = nullptr;
            break;
        }
    }

exit:
    ChipLogFunctError(err);

    if (nullptr != apExchangeContext)
    {
        apExchangeContext->Abort();
    }
}

void InteractionModelEngine::OnMessageReceived(Messaging::ExchangeContext * apExchangeContext, const PacketHeader & aPacketHeader,
                                               const PayloadHeader & aPayloadHeader, System::PacketBufferHandle aPayload)
{
    if (aPayloadHeader.HasMessageType(Protocols::InteractionModel::MsgType::InvokeCommandRequest))
    {
        OnInvokeCommandRequest(apExchangeContext, aPacketHeader, aPayloadHeader, std::move(aPayload));
    }
    else if (aPayloadHeader.HasMessageType(Protocols::InteractionModel::MsgType::ReadRequest))
    {
        OnReadRequest(apExchangeContext, aPacketHeader, aPayloadHeader, std::move(aPayload));
    }
    else
    {
        OnUnknownMsgType(apExchangeContext, aPacketHeader, aPayloadHeader, std::move(aPayload));
    }
}

void InteractionModelEngine::OnResponseTimeout(Messaging::ExchangeContext * ec)
{
    ChipLogProgress(DataManagement, "Time out! failed to receive echo response from Exchange: %d", ec->GetExchangeId());
}

// The default implementation to make compiler happy before codegen for this is ready.
// TODO: Remove this after codegen is ready.
void __attribute__((weak))
DispatchSingleClusterCommand(chip::ClusterId aClusterId, chip::CommandId aCommandId, chip::EndpointId aEndPointId,
                             chip::TLV::TLVReader & aReader, Command * apCommandObj)
{
    ChipLogDetail(DataManagement, "Received Cluster Command: Cluster=%" PRIx16 " Command=%" PRIx8 " Endpoint=%" PRIx8, aClusterId,
                  aCommandId, aEndPointId);
    ChipLogError(
        DataManagement,
        "Default DispatchSingleClusterCommand is called, this should be replaced by actual dispatched for cluster commands");
}

CHIP_ERROR __attribute__((weak)) ReadSingleClusterData(AttributePathParams & aAttributePathParams, TLV::TLVWriter & aWriter)
{
    ChipLogDetail(DataManagement,
                  "Received Cluster Command: Cluster=%" PRIx16 " NodeId=%" PRIx64 " Endpoint=%" PRIx8 " FieldId=%" PRIx8
                  " ListIndex=%" PRIx8,
                  aAttributePathParams.mClusterId, aAttributePathParams.mNodeId, aAttributePathParams.mEndpointId,
                  aAttributePathParams.mFieldId, aAttributePathParams.mListIndex);
    ChipLogError(DataManagement,
                 "Default ReadSingleClusterData is called, this should be replaced by actual dispatched for cluster");
    return CHIP_NO_ERROR;
}

CHIP_ERROR __attribute__((weak)) WriteSingleClusterData(AttributePathParams & aAttributePathParams, TLV::TLVReader & aReader)
{
    ChipLogDetail(DataManagement,
                  "Received Cluster Attribute: Cluster=%" PRIx16 " NodeId=%" PRIx64 " Endpoint=%" PRIx8 " FieldId=%" PRIx8,
                  " ListIndex=%" PRIx8, aAttributePathParams.mClusterId, aAttributePathParams.mNodeId,
                  aAttributePathParams.mEndpointId, aAttributePathParams.mFieldId, aAttributePathParams.mListIndex);
    ChipLogError(DataManagement,
                 "Default WriteSingleClusterData is called, this should be replaced by actual dispatched for cluster");
    return CHIP_NO_ERROR;
}

uint16_t InteractionModelEngine::GetReadClientArrayIndex(const ReadClient * const apReadClient) const
{
    return static_cast<uint16_t>(apReadClient - mReadClients);
}

void InteractionModelEngine::ReleaseClusterInfoList(ClusterInfo *& aClusterInfo)
{
    ClusterInfo * lastClusterInfo = aClusterInfo;
    if (lastClusterInfo == nullptr)
    {
        return;
    }

    while (lastClusterInfo != nullptr && lastClusterInfo->mpNext != nullptr)
    {
        lastClusterInfo->ClearDirty();
        lastClusterInfo = lastClusterInfo->mpNext;
    }
    lastClusterInfo->ClearDirty();
    lastClusterInfo->mpNext    = mpNextAvailableClusterInfo;
    mpNextAvailableClusterInfo = aClusterInfo;
    aClusterInfo               = nullptr;
}

CHIP_ERROR InteractionModelEngine::PushFront(ClusterInfo *& aClusterInfo, AttributePathParams & aAttributePathParams)
{
    ClusterInfo * last = aClusterInfo;
    if (mpNextAvailableClusterInfo == nullptr)
    {
        return CHIP_ERROR_NO_MEMORY;
    }
    aClusterInfo                       = mpNextAvailableClusterInfo;
    mpNextAvailableClusterInfo         = mpNextAvailableClusterInfo->mpNext;
    aClusterInfo->mpNext               = last;
    aClusterInfo->mAttributePathParams = aAttributePathParams;
    return CHIP_NO_ERROR;
}

} // namespace app
} // namespace chip
