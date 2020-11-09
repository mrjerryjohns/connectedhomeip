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

/**
 *    @file
 *      This file implements an object for a CHIP Echo unsolicitied
 *      responder (server).
 *
 */

#include "Echo.h"

namespace chip {
namespace Protocols {

CHIP_ERROR EchoServer::Init(ExchangeManager * exchangeMgr)
{
    // Error if already initialized.
    if (mExchangeMgr != nullptr)
        return CHIP_ERROR_INCORRECT_STATE;

    mExchangeMgr          = exchangeMgr;
    OnEchoRequestReceived = nullptr;

    // Register to receive unsolicited Echo Request messages from the exchange manager.
    mExchangeMgr->RegisterUnsolicitedMessageHandler(kProtocol_Echo, kEchoMessageType_EchoRequest, this);

    return CHIP_NO_ERROR;
}

void EchoServer::Shutdown()
{
    if (mExchangeMgr != nullptr)
    {
        mExchangeMgr->UnregisterUnsolicitedMessageHandler(kProtocol_Echo, kEchoMessageType_EchoRequest);
        mExchangeMgr = nullptr;
    }
}

void EchoServer::EchoSessionStyle1::OnMessageReceived(ExchangeContext * ec, const PacketHeader & packetHeader, uint32_t protocolId, uint8_t msgType,
                                   System::PacketBuffer * payload)
{
    // Since we are re-using the inbound EchoRequest buffer to send the EchoResponse, if necessary,
    // adjust the position of the payload within the buffer to ensure there is enough room for the
    // outgoing network headers.  This is necessary because in some network stack configurations,
    // the incoming header size may be smaller than the outgoing size.
    payload->EnsureReservedSize(CHIP_SYSTEM_CONFIG_HEADER_RESERVE_SIZE);

    // Send an Echo Response back to the sender.
    ec->SendMessage(kProtocol_Echo, kEchoMessageType_EchoResponse, payload);

    // Discard the exchange context.
    ec->Close();

    // We're done, let's free up our session.
    mIsFree = true;
}

void EchoServer::EchoSessionStyle1::OnResponseTimeout(ExchangeContext *ec)
{
    // We're not the initiating node for an echo, so do nothing.
}

void EchoServer::EchoSessionStyle2::OnMessageReceived(ExchangeContext * ec, const PacketHeader & packetHeader, System::PacketBuffer * payload)
{
    // Since we are re-using the inbound EchoRequest buffer to send the EchoResponse, if necessary,
    // adjust the position of the payload within the buffer to ensure there is enough room for the
    // outgoing network headers.  This is necessary because in some network stack configurations,
    // the incoming header size may be smaller than the outgoing size.
    payload->EnsureReservedSize(CHIP_SYSTEM_CONFIG_HEADER_RESERVE_SIZE);

    // Send an Echo Response back to the sender.
    ec->SendMessage(kProtocol_Echo, kEchoMessageType_EchoResponse, payload);

    // Discard the exchange context.
    ec->Close();

    // We're done, let's free up our session.
    mIsFree = true;
}

void EchoServer::EchoSessionStyle2::OnResponseTimeout(ExchangeContext *ec)
{
    // We're not the initiating node for an echo, so do nothing.
}


void EchoServer::OnMessageReceived(ExchangeContext * ec, const PacketHeader & packetHeader, uint32_t protocolId, uint8_t msgType,
                                   System::PacketBuffer * payload)
{

    //
    // Style 1: Allocate a session object, and set it to be the new EC delegate.
    // Style 2: Allocate a session objet, but still have the echo server be the 'router' to route message to the session object.
    // Style 3: No session object, the server handles all EC methods (most common).
    // 
   
    // NOTE: we already know this is an Echo Request message because we explicitly registered with the
    // Exchange Manager for unsolicited Echo Requests.

    // Call the registered OnEchoRequestReceived handler, if any.
    if (OnEchoRequestReceived != nullptr)
        OnEchoRequestReceived(ec->GetPeerNodeId(), payload);

#define STYLE_1 1

#if STYLE_1
    EchoSessionStyle1 *session = AllocEchoSession1();
    if (!session) {
        return;
    }

    // Subsequent messages on this exchange (which are not going to happen for a simple protocol like Echo unfortunately)
    // will come through the session object now.
    ec->SetDelegate(session);

    // For this first message, route it appropriately.
    session->OnMessageReceived(ec, packetHeader, protocolId, msgType, payload);

#elif STYLE_2
    EchoSessionStyle2 *session = AllocEchoSession2();
    session->OnMessageReceived(ec, packetHeader, payload);
#else
    // Since we are re-using the inbound EchoRequest buffer to send the EchoResponse, if necessary,
    // adjust the position of the payload within the buffer to ensure there is enough room for the
    // outgoing network headers.  This is necessary because in some network stack configurations,
    // the incoming header size may be smaller than the outgoing size.
    payload->EnsureReservedSize(CHIP_SYSTEM_CONFIG_HEADER_RESERVE_SIZE);

    // Send an Echo Response back to the sender.
    ec->SendMessage(kProtocol_Echo, kEchoMessageType_EchoResponse, payload);

    // Discard the exchange context.
    ec->Close();
#endif
}

EchoServer::EchoSessionStyle1* EchoServer::AllocEchoSession1()
{
    for (int i = 0; i < 10; i++) {
        if (mSession1Store[i].mIsFree) {
            mSession1Store[i].mIsFree = false;
            return &mSession1Store[i];
        }
    }

    return NULL;
}

void EchoServer::FreeEchoSession1(EchoServer::EchoSessionStyle1 *session)
{
    session->mIsFree = true;
}

EchoServer::EchoSessionStyle2* EchoServer::AllocEchoSession2()
{
    for (int i = 0; i < 10; i++) {
        if (mSession2Store[i].mIsFree) {
            mSession2Store[i].mIsFree = false;
            return &mSession2Store[i];
        }
    }

    return NULL;
}

void EchoServer::FreeEchoSession2(EchoServer::EchoSessionStyle2 *session)
{
    session->mIsFree = true;
}

} // namespace Protocols
} // namespace chip
