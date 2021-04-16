/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <platform/CHIPDeviceLayer.h>
#include <platform/PlatformManager.h>

#include "af.h"
#include "core/CHIPTLVTags.h"
#include "gen/attribute-id.h"
#include "gen/cluster-id.h"
#include <app/chip-zcl-zpro-codec.h>
#include <app/util/af-types.h>
#include <app/util/attribute-storage.h>
#include <app/util/util.h>
#include <core/CHIPError.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <setup_payload/SetupPayload.h>
#include <support/CHIPMem.h>
#include <support/RandUtils.h>
#include <core/CHIPTLVText.hpp>
#include <SchemaTypes.h>
#include <TestCluster-Gen.h>
#include "LightingManager.h"
#include "Options.h"
#include "Server.h"
#include "system/SystemPacketBuffer.h"
#include <system/SystemPacketBuffer.h>
#include <system/TLVPacketBufferBackingStore.h>

#if defined(PW_RPC_ENABLED)
#include "Rpc.h"
#endif // PW_RPC_ENABLED

#include <cassert>
#include <iostream>

using namespace chip;
using namespace chip::Inet;
using namespace chip::Transport;
using namespace chip::DeviceLayer;

void emberAfPostAttributeChangeCallback(EndpointId endpoint, ClusterId clusterId, AttributeId attributeId, uint8_t mask,
                                        uint16_t manufacturerCode, uint8_t type, uint8_t size, uint8_t * value)
{
    if (clusterId != ZCL_ON_OFF_CLUSTER_ID)
    {
        ChipLogProgress(Zcl, "Unknown cluster ID: %d", clusterId);
        return;
    }

    if (attributeId != ZCL_ON_OFF_ATTRIBUTE_ID)
    {
        ChipLogProgress(Zcl, "Unknown attribute ID: %d", attributeId);
        return;
    }

    if (*value)
    {
        LightingMgr().InitiateAction(LightingManager::ON_ACTION);
    }
    else
    {
        LightingMgr().InitiateAction(LightingManager::OFF_ACTION);
    }
}

/** @brief OnOff Cluster Init
 *
 * This function is called when a specific cluster is initialized. It gives the
 * application an opportunity to take care of cluster initialization procedures.
 * It is called exactly once for each endpoint where cluster is present.
 *
 * @param endpoint   Ver.: always
 *
 * TODO Issue #3841
 * emberAfOnOffClusterInitCallback happens before the stack initialize the cluster
 * attributes to the default value.
 * The logic here expects something similar to the deprecated Plugins callback
 * emberAfPluginOnOffClusterServerPostInitCallback.
 *
 */
void emberAfOnOffClusterInitCallback(EndpointId endpoint)
{
    // TODO: implement any additional Cluster Server init actions
}

namespace {
void EventHandler(const chip::DeviceLayer::ChipDeviceEvent * event, intptr_t arg)
{
    (void) arg;
    if (event->Type == chip::DeviceLayer::DeviceEventType::kCHIPoBLEConnectionEstablished)
    {
        ChipLogProgress(DeviceLayer, "Receive kCHIPoBLEConnectionEstablished");
    }
}

CHIP_ERROR PrintQRCodeContent()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    // If we do not have a discriminator, generate one
    chip::SetupPayload payload;
    uint32_t setUpPINCode;
    uint16_t setUpDiscriminator;
    uint16_t vendorId;
    uint16_t productId;
    std::string result;

    err = ConfigurationMgr().GetSetupPinCode(setUpPINCode);
    SuccessOrExit(err);

    err = ConfigurationMgr().GetSetupDiscriminator(setUpDiscriminator);
    SuccessOrExit(err);

    err = ConfigurationMgr().GetVendorId(vendorId);
    SuccessOrExit(err);

    err = ConfigurationMgr().GetProductId(productId);
    SuccessOrExit(err);

    payload.version       = 0;
    payload.vendorID      = vendorId;
    payload.productID     = productId;
    payload.setUpPINCode  = setUpPINCode;
    payload.discriminator = setUpDiscriminator;

    // Wrap it so SuccessOrExit can work
    {
        chip::QRCodeSetupPayloadGenerator generator(payload);
        err = generator.payloadBase41Representation(result);
        SuccessOrExit(err);
    }

    std::cout << "SetupPINCode: [" << setUpPINCode << "]" << std::endl;
    // There might be whitespace in setup QRCode, add brackets to make it clearer.
    std::cout << "SetupQRCode:  [" << result << "]" << std::endl;

exit:
    if (err != CHIP_NO_ERROR)
    {
        std::cerr << "Failed to generate QR Code: " << ErrorStr(err) << std::endl;
    }
    return err;
}
} // namespace

void DumpOffsets(chip::Span<const chip::IM::FieldDescriptor> pStruct, int tabOffset)
{
    for (auto i = pStruct.data(); i != (pStruct.data() + pStruct.size()); i++) {
        for (int j = 0; j < tabOffset; j++) {
            printf("\t");
        }

        printf("[%d] Type: %d (%d)\n", i->FieldId, i->Type.Raw(), i->Offset);

        if ((i->Type.Has(chip::IM::Type::TYPE_STRUCT)) || (i->Type.Has(chip::IM::Type::TYPE_ARRAY))) {
            DumpOffsets(i->StructDef, tabOffset + 1);
        }
    }
}

template <typename T>
void DumpOffsets(T &val)
{
    DumpOffsets({val.mDescriptor.FieldList.data(), val.mDescriptor.FieldList.size()}, 0);
}

int main(int argc, char * argv[])
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    err = chip::Platform::MemoryInit();
    SuccessOrExit(err);

    err = ParseArguments(argc, argv);
    SuccessOrExit(err);

    err = chip::DeviceLayer::PlatformMgr().InitChipStack();
    SuccessOrExit(err);

    err = PrintQRCodeContent();
    SuccessOrExit(err);

#if defined(PW_RPC_ENABLED)
    chip::rpc::Init();
    std::cerr << "PW_RPC initialized." << std::endl;
#endif // defined(PW_RPC_ENABLED)

    chip::DeviceLayer::PlatformMgrImpl().AddEventHandler(EventHandler, 0);

    chip::DeviceLayer::ConnectivityMgr().SetBLEDeviceName(nullptr); // Use default device name (CHIP-XXXX)

#if CONFIG_NETWORK_LAYER_BLE
    chip::DeviceLayer::Internal::BLEMgrImpl().ConfigureBle(LinuxDeviceOptions::GetInstance().mBleDevice, false);
#endif

    chip::DeviceLayer::ConnectivityMgr().SetBLEAdvertisingEnabled(true);

    LightingMgr().Init();

    // Init ZCL Data Model and CHIP App Server
    InitServer();

#if CHIP_DEVICE_CONFIG_ENABLE_WPA
    if (LinuxDeviceOptions::GetInstance().mWiFi)
    {
        chip::DeviceLayer::ConnectivityMgrImpl().StartWiFiManagement();
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_WPA

#if CHIP_ENABLE_OPENTHREAD
    if (LinuxDeviceOptions::GetInstance().mThread)
    {
        SuccessOrExit(err = chip::DeviceLayer::ThreadStackMgrImpl().InitThreadStack());
        std::cerr << "Thread initialized." << std::endl;
    }
#endif // CHIP_ENABLE_OPENTHREAD

    {
        chip::System::PacketBufferHandle bufHandle = System::PacketBufferHandle::New(1024);
        chip::System::TLVPacketBufferBackingStore store;
        chip::TLV::TLVWriter writer;
        chip::TLV::TLVReader reader;

        store.Init(std::move(bufHandle));
        writer.Init(store);

        chip::IM::Cluster::TestCluster::Attributes::Type t;
        DumpOffsets(t);

        uint8_t buf[] = {1, 2, 3, 4, 5, 100, 100, 202, 103};

        t.a = 20;
        t.b = 30;
        t.c = 200000;
        t.d = 1;
        t.e = Span{buf};
        t.f.x = 11;
        t.f.y = 12;

        t.f.z.x = 22;
        t.f.z.y = 33;

        chip::IM::EncodeSchemaElement(t, writer, TLV::AnonymousTag);

        writer.Finalize();
        reader.Init(store);

        chip::TLV::Utilities::Print(reader);
    }
    
    chip::DeviceLayer::PlatformMgr().RunEventLoop();

exit:
    if (err != CHIP_NO_ERROR)
    {
        std::cerr << "Failed to run Linux Lighting App: " << ErrorStr(err) << std::endl;
        // End the program with non zero error code to indicate a error.
        return 1;
    }
    return 0;
}
