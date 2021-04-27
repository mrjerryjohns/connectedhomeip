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

#include "TestCluster.h"
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

nlTestSuite *gpSuite = nullptr;
InvokeInteraction *gServerInvoke = nullptr;
Messaging::ExchangeContext *gClientEc = nullptr;

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
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::app::Cluster::TestCluster::CommandA::Type req;
    uint8_t d[5];

    req.d = chip::Span{d};

    if (commandParams.CommandId == chip::app::Cluster::TestCluster::kCommandAId) {
        printf("Received CommandA\n");

        // 
        // To prevent the stack from actually sending this message
        //
        invokeInteraction.IncrementHoldoffRef();

        gServerInvoke = &invokeInteraction;

        NL_TEST_ASSERT(gpSuite, payload != nullptr); 

        err = DecodeSchemaElement(req, *payload);
        NL_TEST_ASSERT(gpSuite, err == CHIP_NO_ERROR);

        NL_TEST_ASSERT(gpSuite, req.a == 10);
        NL_TEST_ASSERT(gpSuite, req.b == 20);
        NL_TEST_ASSERT(gpSuite, req.c.x == 13);
        NL_TEST_ASSERT(gpSuite, req.c.y == 99);

        for (size_t i = 0; i < std::size(d); i++) {
            NL_TEST_ASSERT(gpSuite, d[i] == i);
        }

        //
        // Send response synchronously
        //
        
        {
            chip::app::Cluster::TestCluster::CommandB::Type resp;
            chip::app::Cluster::TestCluster::StructA::Type e[5];

            resp.a = 21;
            resp.b = 49;
            resp.c.x = 19;
            resp.c.y = 233;
            resp.d = chip::Span{d};
            resp.e = chip::Span{e};
    
            for (size_t i = 0; i < std::size(d); i++) {
                d[i] = (uint8_t)(255 - i);
            }

            for (size_t i = 0; i < std::size(e); i++) {
                e[i].x = (uint8_t)(255 - i);
                e[i].y = (uint8_t)(255 - i);
            }

            commandParams.CommandId = chip::app::Cluster::TestCluster::kCommandBId;
            err = invokeInteraction.AddCommand(commandParams, [&](chip::TLV::TLVWriter &writer, uint64_t tag) {
                return EncodeSchemaElement(resp, writer, tag);
            });

            NL_TEST_ASSERT(gpSuite, err == CHIP_NO_ERROR);
        }
    }

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
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::app::Cluster::TestCluster::CommandB::Type resp;
    uint8_t d[5];
    chip::app::Cluster::TestCluster::StructA::Type e[5];

    resp.d = chip::Span{d};
    resp.e = chip::Span{e};

    if (commandParams.CommandId == chip::app::Cluster::TestCluster::kCommandBId) {
        printf("Received CommandB\n");

        // 
        // To prevent the stack from actually sending this message
        //
        invokeInteraction.IncrementHoldoffRef();

        NL_TEST_ASSERT(gpSuite, payload != nullptr); 

        err = DecodeSchemaElement(resp, *payload);
        NL_TEST_ASSERT(gpSuite, err == CHIP_NO_ERROR);

        NL_TEST_ASSERT(gpSuite, resp.a == 21);
        NL_TEST_ASSERT(gpSuite, resp.b == 49);
        NL_TEST_ASSERT(gpSuite, resp.c.x == 19);
        NL_TEST_ASSERT(gpSuite, resp.c.y == 233);

        for (size_t i = 0; i < std::size(d); i++) {
            NL_TEST_ASSERT(gpSuite, d[i] == (uint8_t)(255 - i));
        }

        for (size_t i = 0; i < std::size(e); i++) {
            NL_TEST_ASSERT(gpSuite, e[i].x == (uint8_t)(255 - i));
            NL_TEST_ASSERT(gpSuite, e[i].y == (uint8_t)(255 - i));
        }
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR TestClientCluster::SendCommand(InvokeInteraction **apInvoke)
{
    chip::app::Cluster::TestCluster::CommandA::Type c;
    InvokeInteraction::CommandParams p = BuildParams();
    InvokeInteraction *invoke;
    CHIP_ERROR err = CHIP_NO_ERROR;
    uint8_t d[5];

    c.a = 10;
    c.b = 20;
    c.c.x = 13;
    c.c.y = 99;
    c.d = chip::Span{d};

    for (size_t i = 0; i < std::size(d); i++) {
        d[i] = (uint8_t)i;
    }
    
    p.CommandId = chip::app::Cluster::TestCluster::kCommandAId;

    invoke = StartInvoke();
    SuccessOrExit(err);

    gClientEc = invoke->GetExchange();

    invoke->IncrementHoldoffRef();
    *apInvoke = invoke;

    err = invoke->AddCommand(p, [&c](chip::TLV::TLVWriter &writer, uint64_t tag) {
        return EncodeSchemaElement(c, writer, tag);
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

void TestInvokeInteraction::TestInvokeInteractionSimple(nlTestSuite * apSuite, void * apContext)
{
    InvokeInteraction *invoke;
    CHIP_ERROR err = CHIP_NO_ERROR;
    System::PacketBufferHandle buf;
    Messaging::ExchangeContext *pRxEc;
    PacketHeader packetHdr;
    PayloadHeader payloadHdr;

    TestClientCluster client;
    TestServerCluster server;

    pRxEc = chip::gExchangeManager.NewContext({0, 0, 0}, NULL);
    NL_TEST_ASSERT(apSuite, pRxEc != nullptr);

    err = chip::app::InteractionModelEngine::GetInstance()->RegisterServer(&server);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = chip::app::InteractionModelEngine::GetInstance()->RegisterClient(&client);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    
    err = client.SendCommand(&invoke);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    err = invoke->FinalizeMessage(buf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);
    
    {
        chip::System::PacketBufferTLVReader reader;
        reader.Init(std::move(buf));
        chip::TLV::Utilities::Print(reader);
        buf = reader.GetBackingStore().Release();
    }

    gServerInvoke = nullptr;

    chip::app::InteractionModelEngine::GetInstance()->OnInvokeCommandRequest(pRxEc, packetHdr, payloadHdr, std::move(buf));
    NL_TEST_ASSERT(apSuite, gServerInvoke != nullptr);

    err = gServerInvoke->FinalizeMessage(buf);
    NL_TEST_ASSERT(apSuite, err == CHIP_NO_ERROR);

    {
        chip::System::PacketBufferTLVReader reader;
        reader.Init(std::move(buf));
        chip::TLV::Utilities::Print(reader);
        buf = reader.GetBackingStore().Release();
    }

    NL_TEST_ASSERT(apSuite, gClientEc != nullptr);
    printf("gClientEc = %lx\n", (uintptr_t)gClientEc);
    chip::app::InteractionModelEngine::GetInstance()->OnInvokeCommandRequest(gClientEc, packetHdr, payloadHdr, std::move(buf));
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

    chip::app::gpSuite = apSuite;

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
