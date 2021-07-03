#pragma once

#include <array>
#include <type_traits>
#include "OperationalCredentialCluster.h"
#include <vector>
#include <string>

namespace chip::app::Cluster::OperationalCredentialCluster {
    namespace AddOpCert {
        class Type : public ISerializable {
            public:
                std::vector<uint8_t> noc;
                std::vector<uint8_t> icaCertificate;
                std::vector<uint8_t> ipkValue;
                chip::NodeId caseAdminNode;
                int16_t adminVendorId;

                static ClusterId_t GetClusterId() { return kClusterId; }
                static uint16_t GetCommandId() { return 0x06; }

            public:
                CHIP_ERROR Encode(TLV::TLVWriter &writer, uint64_t tag) final; 
                CHIP_ERROR Decode(TLV::TLVReader &reader) final;
        };
    }
    
    namespace OpCertResponse {
        class Type : public ISerializable {
            public:
                int8_t statusCode;
                int64_t fabricIndex;
                std::string debugText;

                static ClusterId_t GetClusterId() { return kClusterId; }
                static uint16_t GetCommandId() { return 0x06; }
                
            public:
                CHIP_ERROR Encode(TLV::TLVWriter &writer, uint64_t tag) final; 
                CHIP_ERROR Decode(TLV::TLVReader &reader) final;
        };
    }
}
