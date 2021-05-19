/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <inttypes.h>

#include <app/server/Server.h>

#include <app/InteractionModelEngine.h>
#include <app/server/EchoHandler.h>
#include <app/server/RendezvousServer.h>
#include <app/server/StorablePeerConnection.h>
#include <app/util/DataModelHandler.h>

using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::Transport;
using namespace ::chip::DeviceLayer;
using namespace ::chip::Messaging;

CHIP_ERROR AddChipServerClusters()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    
}
