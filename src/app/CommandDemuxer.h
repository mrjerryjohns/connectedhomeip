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
#include "InvokeInteraction.h"
#include "SchemaTypes.h"
#include "basic-types.h"
#include "messaging/ExchangeDelegate.h"
#include "protocols/secure_channel/Constants.h"
#include <functional>
#include <vector>

namespace chip {
namespace app {

class CommandDemuxer : public InvokeInitiator::ICommandHandler {
public :
   CommandDemuxer(InvokeInitiator &initiator) : mInitiator(initiator) {}

   template <typename T>
   CHIP_ERROR AddCommand(ISerializable *request, CommandParams params,
                  std::function<void (InvokeInitiator&, CommandParams&, T* t1)> onDataFunc = {}, 
                  std::function<void (CHIP_ERROR error, StatusResponse *response)> onErrorFunc = {}) {
        auto onDataClosure = [onDataFunc](InvokeInitiator& initiator, CommandParams &params, TLV::TLVReader *reader) {
            if (reader) {
                T t1;
                t1.Decode(*reader);
                onDataFunc(initiator, params, &t1);
            }
            else {
                onDataFunc(initiator, params, NULL);
            }
        };

        printf("Command ID %02x\n", T::GetCommandId());
        mHandlers.push_back({onDataClosure, onErrorFunc, T::GetClusterId(), T::GetCommandId()});
        return mInitiator.AddSRequest(params, request);
   }

   ~CommandDemuxer() {}

   void HandleResponse(CommandParams &params, InvokeInitiator &initiator, TLV::TLVReader *payload) final {
       for (auto iter : mHandlers) {
           if (iter.clusterId == params.ClusterId && iter.commandId == params.CommandId) {
               iter.onDataClosure(initiator, params, payload);
           }
       }
   }

   void HandleError(CommandParams &params, CHIP_ERROR error, StatusResponse *statusResponse) final {
       for (auto iter : mHandlers) {
           if (iter.clusterId == params.ClusterId && iter.commandId == params.CommandId) {
               iter.onErrorFunc(error, statusResponse);
           }
       }
   }       

private:
    struct Handler {
        std::function<void (InvokeInitiator&, CommandParams &params, TLV::TLVReader *reader)> onDataClosure; 
        std::function<void (CHIP_ERROR error, StatusResponse *response)> onErrorFunc;
        ClusterId_t clusterId;
        uint16_t commandId;
    };

   InvokeInitiator &mInitiator;
   std::vector<Handler> mHandlers;
};

} // namespace app
} // namespace chip
