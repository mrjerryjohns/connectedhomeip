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

class DemuxedInvokeInitiator : public InvokeInitiator::ICommandHandler {
public :
   typedef std::function<void (DemuxedInvokeInitiator& demuxedInitiator)> onDoneFuncDef;

   DemuxedInvokeInitiator(onDoneFuncDef onDoneFunc) { mOnDoneFunc = onDoneFunc; }

   CHIP_ERROR Init(Messaging::ExchangeManager *apExchangeMgr, NodeId aNodeId, Transport::AdminId aAdminId, SecureSessionHandle *secureSession) {
       return mInitiator.Init(apExchangeMgr, this, aNodeId, aAdminId, secureSession);
   }

   template <typename T>
   CHIP_ERROR AddCommand(ISerializable *request, CommandParams params,
                  std::function<void (DemuxedInvokeInitiator &invokeInitiator, CommandParams&, T* t1)> onDataFunc, 
                  std::function<void (DemuxedInvokeInitiator &invokeInitiator, CHIP_ERROR error, StatusResponse *response)> onErrorFunc = {}) {
        auto onDataClosure = [onDataFunc](DemuxedInvokeInitiator& initiator, CommandParams &params, TLV::TLVReader *reader) {
            if (reader) {
                T t1;
                t1.Decode(*reader);
                onDataFunc(initiator, params, &t1);
            }
            else {
                onDataFunc(initiator, params, NULL);
            }
        };

        mHandlers.push_back({onDataClosure, onErrorFunc, T::GetClusterId(), T::GetCommandId()});
        return mInitiator.AddSRequest(params, request);
   }

   CHIP_ERROR AddCommand(ISerializable *request, CommandParams params,
                  std::function<void (DemuxedInvokeInitiator& invokeInitiator, CommandParams&)> onDataFunc = {}, 
                  std::function<void (DemuxedInvokeInitiator& invokeInitiator, CHIP_ERROR error, StatusResponse *response)> onErrorFunc = {}) {
        auto onDataClosure = [onDataFunc](DemuxedInvokeInitiator& initiator, CommandParams &params, TLV::TLVReader *reader) {
            onDataFunc(initiator, params);
        };

        mHandlers.push_back({onDataClosure, onErrorFunc, params.ClusterId, params.CommandId});
        return mInitiator.AddSRequest(params, request);
   }
   
   ~DemuxedInvokeInitiator() {}

   void HandleResponse(InvokeInitiator &initiator, CommandParams &params, TLV::TLVReader *payload) final {
       bool foundMatch = false;

       for (auto iter : mHandlers) {
           if (iter.clusterId == params.ClusterId && iter.commandId == params.CommandId) {
               iter.onDataClosure(*this, params, payload);
               foundMatch = true;
           }
       }

       if (!foundMatch) {
           ChipLogProgress(DataManagement, "Could not find a matching demuxed handler for command! (ClusterId = %04x, Endpoint = %lu, Command = %lu)", 
                           params.ClusterId, (unsigned long)params.EndpointId, (unsigned long)params.CommandId);
       }           
   }

   void HandleError(InvokeInitiator &initiator, CommandParams *params, CHIP_ERROR error, StatusResponse *statusResponse) final {
       for (auto iter : mHandlers) {
           if ((params && (iter.clusterId == params->ClusterId && iter.commandId == params->CommandId)) || (!params)) {
               iter.onErrorFunc(*this, error, statusResponse);
           }
       }
   }

   CHIP_ERROR Send() {
       return mInitiator.Send();
   }

   void OnEnd(InvokeInitiator &initiator) final {
        mOnDoneFunc(*this);
   }

   InvokeInitiator &GetInitiator() { return mInitiator; }

private:
    struct Handler {
        std::function<void (DemuxedInvokeInitiator& invokeInitiator, CommandParams &params, TLV::TLVReader *reader)> onDataClosure; 
        std::function<void (DemuxedInvokeInitiator& invokeInitiator, CHIP_ERROR error, StatusResponse *response)> onErrorFunc;
        ClusterId_t clusterId;
        chip::CommandId commandId;
    };

   InvokeInitiator mInitiator;
   std::vector<Handler> mHandlers;
   onDoneFuncDef mOnDoneFunc;
};

} // namespace app
} // namespace chip
