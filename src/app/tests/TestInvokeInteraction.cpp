/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
 *      This file implements unit tests for CHIP Interaction Model Command Interaction
 *
 */

#include <app/InteractionModelEngine.h>
#include <core/CHIPCore.h>
#include <core/CHIPTLV.h>
#include <core/CHIPTLVText.hpp>
#include <core/CHIPTLVData.hpp>
#include <core/CHIPTLVDebug.hpp>
#include <core/CHIPTLVUtilities.hpp>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeMgr.h>
#include <messaging/Flags.h>
#include <platform/CHIPDeviceLayer.h>
#include <protocols/secure_channel/PASESession.h>
#include <support/ErrorStr.h>
#include <support/UnitTestRegistration.h>
#include <system/SystemPacketBuffer.h>
#include <system/TLVPacketBufferBackingStore.h>
#include <transport/SecureSessionMgr.h>
#include <transport/raw/UDP.h>

#include <nlunit-test.h>

#include <TestCluster-Gen.h>

namespace chip {
static System::Layer gSystemLayer;
static SecureSessionMgr gSessionManager;
static Messaging::ExchangeManager gExchangeManager;
static TransportMgr<Transport::UDP> gTransportManager;
static Transport::AdminId gAdminId = 0;

namespace app {

class TestServerCluster : public ClusterServer
{
public:
    TestServerCluster();
    CHIP_ERROR HandleCommand(InvokeInteraction::CommandParams &commandParams, InvokeInteraction &invokeInteraction, TLV::TLVReader *payload);
};

TestServerCluster::TestServerCluster()
    : ClusterServer(&chip::app::Cluster::TestCluster::ClusterDescriptor)
{
}

CHIP_ERROR 
TestServerCluster::HandleCommand(InvokeInteraction::CommandParams &commandParams, InvokeInteraction &invokeInteraction, TLV::TLVReader *payload)
{
    return CHIP_NO_ERROR;
}

class TestClientCluster : public ClusterClient
{
public:
    TestClientCluster();
    CHIP_ERROR SendCommand(InvokeInteraction **apInvoke);
    CHIP_ERROR HandleCommand(InvokeInteraction::CommandParams &commandParams, InvokeInteraction &invokeInteraction, TLV::TLVReader *payload);
};

TestClientCluster::TestClientCluster()
    : ClusterClient(&chip::app::Cluster::TestCluster::ClusterDescriptor)
{
}

CHIP_ERROR
TestClientCluster::HandleCommand(InvokeInteraction::CommandParams &commandParams, InvokeInteraction &invokeInteraction, TLV::TLVReader *payload)
{
    return CHIP_NO_ERROR;
}

CHIP_ERROR TestClientCluster::SendCommand(InvokeInteraction **apInvoke)
{
    chip::app::Cluster::TestCluster::StructA::Type t;
    InvokeInteraction::CommandParams p = BuildParams();
    InvokeInteraction *invoke;
    CHIP_ERROR err = CHIP_NO_ERROR;

    t.x = 20;
    t.y = 30;

    p.CommandId = 0x01;

    invoke = StartInvoke();
    SuccessOrExit(err);

    invoke->IncrementHoldoffRef();
    *apInvoke = invoke;

    err = invoke->AddCommand(p, [&t](chip::TLV::TLVWriter &writer, uint64_t tag) {
        return EncodeSchemaElement(t, writer, tag);
    });

exit:
    return err;
}


class TestInvokeInteraction
{
public:
    static void TestInvokeInteractionSimple(nlTestSuite * apSuite, void * apContext);
};

using namespace chip::TLV;

static const uint8_t Encoding1_DataMacro [] =
{
    CHIP_TLV_STRUCTURE(CHIP_TLV_TAG_ANONYMOUS),
        CHIP_TLV_ARRAY(CHIP_TLV_TAG_CONTEXT_SPECIFIC(0)),
            CHIP_TLV_STRUCTURE(CHIP_TLV_TAG_ANONYMOUS),
                CHIP_TLV_PATH(CHIP_TLV_TAG_CONTEXT_SPECIFIC(0)),
                    CHIP_TLV_UINT8(CHIP_TLV_TAG_CONTEXT_SPECIFIC(0), 176),
                    CHIP_TLV_UINT16(CHIP_TLV_TAG_CONTEXT_SPECIFIC(2), 1),
                    CHIP_TLV_UINT8(CHIP_TLV_TAG_CONTEXT_SPECIFIC(3), 1),
                CHIP_TLV_END_OF_CONTAINER,
                CHIP_TLV_STRUCTURE(CHIP_TLV_TAG_CONTEXT_SPECIFIC(1)),
                    CHIP_TLV_UINT8(CHIP_TLV_TAG_CONTEXT_SPECIFIC(0), 20),
                    CHIP_TLV_UINT8(CHIP_TLV_TAG_CONTEXT_SPECIFIC(1), 30),
                CHIP_TLV_END_OF_CONTAINER,
            CHIP_TLV_END_OF_CONTAINER,
        CHIP_TLV_END_OF_CONTAINER,
    CHIP_TLV_END_OF_CONTAINER,
};

#if 0
#define FWD(xs) ::std::forward<decltype(xs)>(xs)

class ErrorManagedReader {
public:
    template <class... Ts, class... TArgs>
    void forwarder(CHIP_ERROR(TLVReader::*fptr)(Ts...), TArgs&&... xs)
    {
        (reader.*fptr)(FWD(xs)...);
    }

    ErrorManagedReader& Add(CHIP_ERROR status) {
        err = status;
        return (*this);
    }

    ErrorManagedReader& Add(void) {
        return (*this);
    }
    

    CHIP_ERROR err;
};
#endif

void TestInvokeInteraction::TestInvokeInteractionSimple(nlTestSuite * apSuite, void * apContext)
{
    InvokeInteraction *invoke;
    CHIP_ERROR err = CHIP_NO_ERROR;
    System::PacketBufferHandle buf;

    TestClientCluster client;

    err = client.SendCommand(&invoke);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = invoke->FinalizeMessage(buf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

#if 0
    {
        uint8_t buf1[sizeof(Encoding1_DataMacro)];
        memcpy(buf1, Encoding1_DataMacro, sizeof(Encoding1_DataMacro));

        TLVReader reader;
        reader.Init(buf1, sizeof(buf1));
        chip::TLV::Utilities::Print(reader);
    }
  
    {
       ErrorManagedReader reader;
       TLVReader reader1;
       const uint8_t *data;
       uint32_t len;
       bool v;
        
       reader.Add(reader1.Init(data, len));
       
    } 
#endif
    
    {
        chip::System::PacketBufferTLVReader reader;
        reader.Init(std::move(buf));

        chip::TLV::Utilities::Print(reader);
    }
}

} // namespace app
} // namespace chip

namespace {

void InitializeChip(nlTestSuite * apSuite)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::Optional<chip::Transport::PeerAddress> peer(chip::Transport::Type::kUndefined);
    chip::Transport::AdminPairingTable admins;
    chip::Transport::AdminPairingInfo * adminInfo = admins.AssignAdminId(chip::gAdminId, chip::kTestDeviceNodeId);

    NL_TEST_ASSERT(apSuite, adminInfo != nullptr);

    err = chip::Platform::MemoryInit();
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    chip::gSystemLayer.Init(nullptr);

    err = chip::gSessionManager.Init(chip::kTestDeviceNodeId, &chip::gSystemLayer, &chip::gTransportManager, &admins);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = chip::gExchangeManager.Init(&chip::gSessionManager);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = chip::app::InteractionModelEngine::GetInstance()->Init(&chip::gExchangeManager, nullptr);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
}

// clang-format off
const nlTest sTests[] =
{
    NL_TEST_DEF("TestInvokeInteractionSimple", chip::app::TestInvokeInteraction::TestInvokeInteractionSimple),
    NL_TEST_SENTINEL()
};
// clang-format on

} // namespace

int TestInvokeInteraction()
{
    // clang-format off
    nlTestSuite theSuite =
	{
        "InvokeInteraction",
        &sTests[0],
        nullptr,
        nullptr
    };
    // clang-format on

    InitializeChip(&theSuite);

    nlTestRunner(&theSuite, nullptr);

    return (nlTestRunnerStats(&theSuite));
}

CHIP_REGISTER_TEST_SUITE(TestInvokeInteraction)
