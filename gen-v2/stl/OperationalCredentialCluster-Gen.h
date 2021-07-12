#pragma once

#include <array>
#include <type_traits>
#include <vector>
#include <string>
#include <basic-types.h>
#include <IEncodableElement.h>

namespace chip {
namespace app {
namespace Cluster {
namespace OperationalCredentialCluster {
    const chip::ClusterId kClusterId = 0x003E;
    
    namespace AddOpCert {
        enum FieldId {
            kFieldIdNoc = 0,
            kFieldIdIpkValue = 1,
            kFieldIdCaseAdminNode = 2,
            kFieldIdAdminVendorId = 3
        };
        
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
        enum FieldId {
            kFieldIdStatusCode = 0,
            kFieldIdFabricIndex = 1,
            kFieldIdDebugText = 2
        };
        
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
        enum FieldID {
            kFieldIdCsrNonce = 0
        };
        
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
        enum FieldID {
            kFieldIdCsr = 0,
            kFieldIdCsrNonce = 1,
            kFieldIdVendorReserved1 = 2,
            kFieldIdVendorReserved2 = 3,
            kFieldIdVendorReserved3 = 4,
            kFieldIdSignature = 5
        };
        
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
