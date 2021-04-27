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
#include <SchemaTypes.h>
#include <functional>

namespace chip {
namespace app {

class InvokeInteraction
{
public:
    enum State
    {
        kStateReady = 0,           //< The invoke command message has been initialized and is ready
        kStateAwaitingResponse = 1
    };

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
       
        CommandParams() : EndpointId(0), GroupId(0), ClusterId(0), CommandId(0), Flags(TargetType::kTargetEndpoint) {}
        CommandParams(chip::EndpointId endpointId, chip::GroupId groupId, chip::ClusterId clusterId, chip::CommandId commandId,
                      const BitFlags<TargetType> & flags) :
            EndpointId(endpointId),
            GroupId(groupId), ClusterId(clusterId), CommandId(commandId), Flags(flags)
        {}

        chip::EndpointId EndpointId;
        chip::GroupId GroupId;
        chip::ClusterId ClusterId;
        chip::CommandId CommandId;
        BitFlags<TargetType> Flags;
    };

    class CommandHandler
    {
    public:
        virtual CHIP_ERROR HandleCommand(InvokeInteraction::CommandParams &commandParams, InvokeInteraction &invokeInteraction, TLV::TLVReader *payload) = 0;
        virtual ~CommandHandler() {} 
    };

    CHIP_ERROR Init(Messaging::ExchangeContext *aExchangeCtx = NULL);
    CHIP_ERROR StartCommandHeader(CommandParams &aParams);

    template <typename F>
    CHIP_ERROR AddCommand(CommandParams &aParams, F f) {
        CHIP_ERROR err = CHIP_NO_ERROR;

        err = StartCommandHeader(aParams);
        SuccessOrExit(err);

        // 
        // Invoke the passed in closure that will actually write out the command payload if any
        //
        err = f(*mInvokeCommandBuilder.GetWriter(), TLV::ContextTag(CommandDataElement::kCsTag_Data));
        SuccessOrExit(err);

        mInvokeCommandBuilder.GetCommandListBuilder().GetCommandDataElementBuidler().EndOfCommandDataElement();
        SuccessOrExit((err = mInvokeCommandBuilder.GetCommandListBuilder().GetCommandDataElementBuidler().GetError()));

        //
        // This will auto-send the message if the hold off count dips back to 0
        //
        err = DecrementHoldoffRef();
        SuccessOrExit(err);

exit:
        return err;
    }

    CHIP_ERROR AddStatus(CommandParams &aParams, uint16_t aCode);
    Messaging::ExchangeContext *GetExchange() { return mpExchangeCtx; }

    void IncrementHoldoffRef();
    CHIP_ERROR DecrementHoldoffRef();
    void Abort();
    
private:
    //
    // Only invoked by InteractionModelEngine
    //
    CHIP_ERROR HandleMessage(System::PacketBufferHandle payload);
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
    int mHoldOffCount = 0;
    Mode mMode = kModeUnset;
    State mState;
    chip::System::PacketBufferTLVWriter mWriter;
    app::InvokeCommand::Builder mInvokeCommandBuilder;
    Messaging::ExchangeContext *mpExchangeCtx = nullptr;
};

} // namespace app
} // namespace chip
