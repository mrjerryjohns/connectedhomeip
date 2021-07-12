#pragma once

#include <array>
#include <type_traits>
#include "OperationalCredentialCluster.h"
#include <vector>
#include <string>
#include <basic-types.h>
#include <IEncodableElement.h>

namespace chip {
namespace app {
namespace Cluster {
namespace OperationalCredentialCluster {
    namespace AddOpCert {
        class Type : public IEncodable {
            public:
                std::vector<uint8_t> noc;
                std::vector<uint8_t> ipkValue;
                chip::NodeId caseAdminNode;
                int16_t adminVendorId;

                static chip::ClusterId GetClusterId() { return kClusterId; }
                static uint16_t GetCommandId() { return 0x06; }

            public:
                CHIP_ERROR Encode(TLV::TLVWriter &writer, uint64_t tag) final; 
                CHIP_ERROR Decode(TLV::TLVReader &reader) final;
        };
    }
    
    namespace OpCertResponse {
        class Type : public IEncodable {
            public:
                int8_t statusCode;
                int64_t fabricIndex;
                std::string debugText;

                static chip::ClusterId GetClusterId() { return kClusterId; }

                //
                // TODO: This SHOULD be 0x08, but because the device side sends back a faulty status 
                // response, we need to switch this back to 0x06.
                //
                static uint16_t GetCommandId() { return 0x06; }
                
            public:
                CHIP_ERROR Encode(TLV::TLVWriter &writer, uint64_t tag) final; 
                CHIP_ERROR Decode(TLV::TLVReader &reader) final;
        };
    }

    namespace OpCsrRequest {
        class Type : public IEncodable {
            public:
                std::vector<uint8_t> csrNonce;

                static chip::ClusterId GetClusterId() { return kClusterId; }
                static uint16_t GetCommandId() { return 0x04; }

            public:
                CHIP_ERROR Encode(TLV::TLVWriter &writer, uint64_t tag) final; 
                CHIP_ERROR Decode(TLV::TLVReader &reader) final;
        };
    }

    namespace OpCsrResponse {
        class Type : public IEncodable {
            public:
                std::vector<uint8_t> csr;
                std::vector<uint8_t> csrNonce;
                std::vector<uint8_t> vendorReserved1;
                std::vector<uint8_t> vendorReserved2;
                std::vector<uint8_t> vendorReserved3;
                std::vector<uint8_t> signature;

                static chip::ClusterId GetClusterId() { return kClusterId; }
                static uint16_t GetCommandId() { return 0x05; }

            public:
                CHIP_ERROR Encode(TLV::TLVWriter &writer, uint64_t tag) final; 
                CHIP_ERROR Decode(TLV::TLVReader &reader) final;
        };
    }
}
}
}
}
