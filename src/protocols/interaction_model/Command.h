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

#include <app/InteractionModelDelegate.h>
#include <app/MessageDef/CommandDataElement.h>
#include <app/MessageDef/CommandList.h>
#include <app/MessageDef/InvokeCommand.h>

namespace chip {
namespace IM {

class Command
{
public:
    enum class State
    {
        kStateUnInitialized = 0, //< The invoke command message has not been initialized
        kStateReady,             //< The invoke command message has been initialized and is ready
        kStateAwaitingResponse, 
    };

    enum class TargetType : uint8_t
    {
        kTargetEndpoint     = 0x01, /**< Set when the EndpointId field is valid */
        kTargetGroup        = 0x02, /**< Set when the GroupId field is valid */
    };

    /**
     * Encapsulates arguments to be passed into SendCommand().
     *
     */
    struct Params
    {
        Params(chip::EndpointId endpointId, chip::GroupId groupId, chip::ClusterId clusterId, chip::CommandId commandId,
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

    /**
     *  Initialize the Command object. Within the lifetime
     *  of this instance, this method is invoked once after object
     *  construction until a call to Shutdown is made to terminate the
     *  instance.
     *
     *  @param[in]    apExchangeMgr    A pointer to the ExchangeManager object.
     *  @param[in]    apDelegate       InteractionModelDelegate set by application.
     *
     *  @retval #CHIP_ERROR_INCORRECT_STATE If the state is not equal to
     *          CommandState::NotInitialized.
     *  @retval #CHIP_NO_ERROR On success.
     *
     */
    CHIP_ERROR Init();

    CHIP_ERROR AddCommandHeader(Params &aParams);
    chip::TLV::TLVWriter& GetCommandWriter();

    CHIP_ERROR AddStatusCode(uint16_t aCode);
    
    CHIP_ERROR FinalizeAndSend();
    
    void Close();
    
    /**
     * Gets the inner exchange context object, without ownership.
     *
     * @return The inner exchange context, might be nullptr if no
     *         exchange context has been assigned or the context
     *         has been released.
     */
    const Messaging::ExchangeContext * GetExchangeContext() const { return mpExchangeCtx; }
    virtual ~Command() = default;
    bool IsFree() const { return (mState == kStateUnInitialized); };

protected:
    virtual CHIP_ERROR OnResponseReceived(Params &aParams, chip::TLV::TLVReader &aPayloadReader) = 0;
    virtual CHIP_ERROR OnStatusReceived(uint16_t aCode) = 0;

protected:
    CHIP_ERROR ClearExistingExchangeContext();
    const char * GetStateStr() const;

    app::InvokeCommand::Builder mInvokeCommandBuilder;
    Messaging::ExchangeContext *mpExchangeCtx = nullptr;
    chip::System::PacketBufferHandle mBuf;
    uint8_t mCommandIndex = 0;

private:
    friend class TestCommandInteraction;
    State mState;
    chip::System::PacketBufferTLVWriter mWriter;
};
} // namespace app
} // namespace chip
