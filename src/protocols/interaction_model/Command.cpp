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

#include "Command.h"
#include <core/CHIPTLVDebug.hpp>

namespace chip {
namespace IM {

CHIP_ERROR Command::Init()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    app::CommandList::Builder commandListBuilder;
    chip::System::PacketBufferHandle buf;

    VerifyOrExit(IsFree(), err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mBuf.IsNull(), err = CHIP_ERROR_INCORRECT_STATE);

exit:
    ChipLogFunctError(err);
    return err;
}

CHIP_ERROR Command::Reset()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    ClearExistingExchangeContext();

    if (mCommandMessageBuf.IsNull())
    {
        // TODO: Calculate the packet buffer size
        mCommandMessageBuf = System::PacketBufferHandle::New(chip::app::kMaxSecureSduLengthBytes);
        VerifyOrExit(!mCommandMessageBuf.IsNull(), err = CHIP_ERROR_NO_MEMORY);
    }

    mCommandMessageWriter.Init(std::move(mCommandMessageBuf));
    err = mInvokeCommandBuilder.Init(&mCommandMessageWriter);
    SuccessOrExit(err);

    commandListBuilder = mInvokeCommandBuilder.CreateCommandListBuilder();
    SuccessOrExit(commandListBuilder.GetError());
    MoveToState(CommandState::Initialized);

    mCommandIndex = 0;

exit:
    ChipLogFunctError(err);

    return err;
}

CHIP_ERROR Command::ProcessCommandMessage(System::PacketBufferHandle && payload, CommandRoleId aCommandRoleId)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferTLVReader reader;
    chip::TLV::TLVReader commandListReader;
    InvokeCommand::Parser invokeCommandParser;
    CommandList::Parser commandListParser;

    reader.Init(std::move(payload));
    err = reader.Next();
    SuccessOrExit(err);

    err = invokeCommandParser.Init(reader);
    SuccessOrExit(err);

    err = invokeCommandParser.CheckSchemaValidity();
    SuccessOrExit(err);

    err = invokeCommandParser.GetCommandList(&commandListParser);
    SuccessOrExit(err);

    commandListParser.GetReader(&commandListReader);

    while (CHIP_NO_ERROR == (err = commandListReader.Next()))
    {
        VerifyOrExit(chip::TLV::AnonymousTag == commandListReader.GetTag(), err = CHIP_ERROR_INVALID_TLV_TAG);
        VerifyOrExit(chip::TLV::kTLVType_Structure == commandListReader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

        CommandDataElement::Parser commandElement;

        err = commandElement.Init(commandListReader);
        SuccessOrExit(err);

        err = commandElement.CheckSchemaValidity();
        SuccessOrExit(err);

        err = ProcessCommandDataElement(commandElement);
        SuccessOrExit(err);
    }

    // if we have exhausted this container
    if (CHIP_END_OF_TLV == err)
    {
        err = CHIP_NO_ERROR;
    }

exit:
    return err;
}

void Command::Shutdown()
{
    VerifyOrExit(mState != CommandState::Uninitialized, );
    mCommandMessageWriter.Reset();
    mCommandMessageBuf = nullptr;

    ClearExistingExchangeContext();

    mpExchangeMgr = nullptr;
    mpDelegate    = nullptr;
    MoveToState(CommandState::Uninitialized);

    mCommandIndex = 0;
exit:
    return;
}

chip::TLV::TLVWriter & Command::CreateCommandDataElementTLVWriter()
{
    mCommandDataBuf = chip::System::PacketBufferHandle::New(chip::app::kMaxSecureSduLengthBytes);
    if (mCommandDataBuf.IsNull())
    {
        ChipLogDetail(DataManagement, "Unable to allocate packet buffer");
    }

    mCommandDataWriter.Init(mCommandDataBuf.Retain());

    return mCommandDataWriter;
}

CHIP_ERROR Command::AddCommand(chip::EndpointId aEndpointId, chip::GroupId aGroupId, chip::ClusterId aClusterId,
                               chip::CommandId aCommandId, BitFlags<CommandPathFlags> aFlags)
{
    CommandParams commandParams(aEndpointId, aGroupId, aClusterId, aCommandId, aFlags);

    return AddCommand(commandParams);
}

CHIP_ERROR Command::AddCommand(CommandParams & aCommandParams)
{
    CHIP_ERROR err              = CHIP_NO_ERROR;
    const uint8_t * commandData = nullptr;
    uint32_t commandLen         = 0;

    if (!mCommandDataBuf.IsNull())
    {
        commandData = mCommandDataBuf->Start();
        commandLen  = mCommandDataBuf->DataLength();
    }

    if (commandLen > 0 && commandData != nullptr)
    {
        // Command argument list can be empty.
        VerifyOrExit(commandLen >= 2, err = CHIP_ERROR_INVALID_ARGUMENT);
        VerifyOrExit(commandData[0] == chip::TLV::kTLVType_Structure, err = CHIP_ERROR_INVALID_ARGUMENT);

        commandData += 1;
        commandLen -= 1;
    }

    {
        CommandDataElement::Builder commandDataElement =
            mInvokeCommandBuilder.GetCommandListBuilder().CreateCommandDataElementBuilder();

        err = Command::ConstructCommandPath(aCommandParams, commandDataElement);
        SuccessOrExit(err);

        if (commandLen > 0 && commandData != nullptr)
        {
            // Copy the application data into a new TLV structure field contained with the
            // command structure.  NOTE: The TLV writer will take care of moving the app data
            // to the correct location within the buffer.
            err = mInvokeCommandBuilder.GetWriter()->PutPreEncodedContainer(chip::TLV::ContextTag(CommandDataElement::kCsTag_Data),
                                                                            chip::TLV::kTLVType_Structure, commandData, commandLen);
            SuccessOrExit(err);
        }
        commandDataElement.EndOfCommandDataElement();

        err = commandDataElement.GetError();
        SuccessOrExit(err);
    }
    MoveToState(CommandState::AddCommand);

exit:
    mCommandDataBuf = nullptr;
    ChipLogFunctError(err);
    return err;
}

CHIP_ERROR Command::ConstructCommandPath(const CommandParams & aCommandParams, CommandDataElement::Builder & aCommandDataElement)
{
    CommandPath::Builder commandPath = aCommandDataElement.CreateCommandPathBuilder();
    if (aCommandParams.Flags.Has(CommandPathFlags::kEndpointIdValid))
    {
        commandPath.EndpointId(aCommandParams.EndpointId);
    }

    if (aCommandParams.Flags.Has(CommandPathFlags::kGroupIdValid))
    {
        commandPath.GroupId(aCommandParams.GroupId);
    }

    commandPath.ClusterId(aCommandParams.ClusterId).CommandId(aCommandParams.CommandId).EndOfCommandPath();

    return commandPath.GetError();
}

CHIP_ERROR Command::ClearExistingExchangeContext()
{
    // Discard any existing exchange context. Effectively we can only have one Echo exchange with
    // a single node at any one time.
    if (mpExchangeCtx != nullptr)
    {
        mpExchangeCtx->Abort();
        mpExchangeCtx = nullptr;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR Command::FinalizeCommandsMessage()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    CommandList::Builder commandListBuilder = mInvokeCommandBuilder.GetCommandListBuilder().EndOfCommandList();
    SuccessOrExit(commandListBuilder.GetError());

    mInvokeCommandBuilder.EndOfInvokeCommand();
    err = mInvokeCommandBuilder.GetError();
    SuccessOrExit(err);

    err = mCommandMessageWriter.Finalize(&mCommandMessageBuf);
    SuccessOrExit(err);

    VerifyOrExit(mCommandMessageBuf->EnsureReservedSize(System::PacketBuffer::kDefaultHeaderReserve),
                 err = CHIP_ERROR_BUFFER_TOO_SMALL);

exit:
    ChipLogFunctError(err);
    return err;
}

const char * Command::GetStateStr() const
{
#if CHIP_DETAIL_LOGGING
    switch (mState)
    {
    case CommandState::Uninitialized:
        return "Uninitialized";

    case CommandState::Initialized:
        return "Initialized";

    case CommandState::AddCommand:
        return "AddCommand";

    case CommandState::Sending:
        return "Sending";
    }
#endif // CHIP_DETAIL_LOGGING
    return "N/A";
}

void Command::MoveToState(const CommandState aTargetState)
{
    mState = aTargetState;
    ChipLogDetail(DataManagement, "ICR moving to [%10.10s]", GetStateStr());
}

void Command::ClearState(void)
{
    MoveToState(CommandState::Uninitialized);
}

} // namespace app
} // namespace chip
