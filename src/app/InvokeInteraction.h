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
#include <functional>

namespace chip {
namespace app {

class InvokeInteraction
{
public:
    enum State
    {
        kStateReady = 0,           //< The invoke command message has been initialized and is ready
        kStateAwaitingResponse = 1,
        kStateReleased = 3         //< This object has now been effectively retired/released, but not actually deleted. There should be no further interactions against this object aside from calling the destructor.
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
       
        chip::EndpointId EndpointId = 0;
        chip::GroupId GroupId = 0;
        chip::ClusterId ClusterId = 0;
        chip::CommandId CommandId = 0;
        BitFlags<TargetType> Flags = TargetType::kTargetEndpoint;
        bool ExpectsResponses = false;
    };

    class CommandHandler
    {
    public:
        virtual CHIP_ERROR HandleCommand(InvokeInteraction::CommandParams &commandParams, InvokeInteraction &invokeInteraction, TLV::TLVReader *payload) = 0;
        virtual ~CommandHandler() {} 
    };

    CHIP_ERROR Init(Messaging::ExchangeContext *aExchangeCtx = NULL, bool aOnCommandRx = false);

    CHIP_ERROR AddStatusCode(const CommandParams &aParams, 
                             const Protocols::SecureChannel::GeneralStatusCode aGeneralCode,
                             const Protocols::Id aProtocolId, const uint16_t aProtocolCode);

    template <typename F>
    CHIP_ERROR AddCommand(CommandParams &aParams, F f) {
        CHIP_ERROR err = CHIP_NO_ERROR;

        IncrementHoldoffRef();

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

        //
        // This will auto-send the message if the hold off count goes from 1 -> 0 (which it should, because
        // of the increment we did earlier in this call)
        //
        err = DecrementHoldoffRef();
        SuccessOrExit(err);

exit:
        return err;
    }

    Messaging::ExchangeContext *GetExchange() { return mpExchangeCtx; }
    void IncrementHoldoffRef();
    CHIP_ERROR DecrementHoldoffRef();
   

private:
    CHIP_ERROR StartCommandHeader(const CommandParams &aParams);

    void ReleaseOnCompletion();
    
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
    bool mExpectsResponses = false;
    State mState = kStateReady;
    chip::System::PacketBufferTLVWriter mWriter;
    app::InvokeCommand::Builder mInvokeCommandBuilder;
    Messaging::ExchangeContext *mpExchangeCtx = nullptr;
};

} // namespace app
} // namespace chip
