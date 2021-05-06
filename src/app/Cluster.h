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
#include "app/util/basic-types.h"
#include <functional>

namespace chip {
namespace app {

class ClusterServer : public InvokeInteraction::CommandHandler
{
public:
    ClusterServer(ClusterDescriptor *apClusterDescriptor);
    virtual ~ClusterServer() = default;

    void SetEndpoint(chip::EndpointId aEndpointId) { mEndpoint = aEndpointId; }

    ClusterId_t GetClusterId() { return mClusterDescriptor->ClusterId; }
    chip::EndpointId GetEndpoint() { return mEndpoint; }

private:
    ClusterDescriptor *mClusterDescriptor;
    chip::EndpointId mEndpoint;
};

class ClusterClient : public InvokeInteraction::CommandHandler
{
public:
    ClusterClient(ClusterDescriptor *apClusterDescriptor);
    InvokeInteraction* StartInvoke();
    virtual ~ClusterClient() = default;

    InvokeInteraction::CommandParams BuildParams() {
        InvokeInteraction::CommandParams params(mRemoteEndpoint, 0, (ClusterId)mClusterDescriptor->ClusterId, 
                                                0, InvokeInteraction::CommandParams::TargetType::kTargetEndpoint);
        return params;
    }

    void SetRemoteTarget(NodeId aRemoteNode, Transport::AdminId aRemoteAdmin) { mRemoteNode = aRemoteNode; mRemoteAdmin = aRemoteAdmin; }
    void SetRemoteEndpoint(chip::EndpointId aEndpointId) { mRemoteEndpoint = aEndpointId; }

    ClusterId_t GetClusterId() {return mClusterDescriptor->ClusterId;}
    chip::EndpointId GetEndpointId() {return mRemoteEndpoint;}
    NodeId GetRemoteNodeId() {return mRemoteNode;}
    Transport::AdminId GetRemoteAdmin() { return mRemoteAdmin; }

private:
    ClusterDescriptor *mClusterDescriptor;
    chip::EndpointId mRemoteEndpoint;
    NodeId mRemoteNode;
    Transport::AdminId mRemoteAdmin;
};

} // namespace app
} // namespace chip