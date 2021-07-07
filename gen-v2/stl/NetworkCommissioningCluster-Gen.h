#pragma once

#include <array>
#include <type_traits>
#include "NetworkCommissioningCluster.h"

namespace chip {
namespace app {
namespace Cluster {
namespace NetworkCommissioningCluster {
    extern ClusterDescriptor ClusterDescriptor;

#if 0
    namespace AddThreadNetworkCommand {
        constexpr int NumImplementedFields = GetNumImplementedFields(_Schema);
        extern const StructDescriptor<NumImplementedFields> Descriptor;
        
        class Type {
            public:
                chip::ByteSpan OperationalDataSet;
                uint64_t Breadcrumb;
                uint32_t TimeoutMs;

                static const StructDescriptor<NumImplementedFields> &mDescriptor;
        };
    }
#endif

    namespace AddThreadNetworkCommand {
        constexpr int NumImplementedFields = GetNumImplementedFields(_Schema);
        extern const StructDescriptor<NumImplementedFields> Descriptor;
        
        class Type : public ISerializable {
            public:
                class Data {
                public:
                    chip::ByteSpan OperationalDataSet;
                    uint64_t Breadcrumb;
                    uint32_t TimeoutMs;
                };

                ClusterId_t GetClusterId() { return kClusterId; }
                uint16_t GetCommandId() { return kAddThreadNetworkRequestCommandId; }

                Data data;

                CHIP_ERROR Encode(TLV::TLVWriter &writer, uint64_t tag) {
                    return EncodeSchemaElement({mDescriptor.FieldList.data(), mDescriptor.FieldList.size()}, &data, tag, writer);
                }

                CHIP_ERROR Decode(TLV::TLVReader &reader) {
                    return DecodeSchemaElement({mDescriptor.FieldList.data(), mDescriptor.FieldList.size()}, &data, reader);
                }
                
                static const StructDescriptor<NumImplementedFields> &mDescriptor;
        };
    }
    
    namespace AddWifiNetworkCommand {
        constexpr int NumImplementedFields = GetNumImplementedFields(_Schema);
        extern const StructDescriptor<NumImplementedFields> Descriptor;
        
        class Type {
            public:
                chip::ByteSpan Ssid;
                chip::ByteSpan Credentials;
                uint64_t Breadcrumb;
                uint32_t TimeoutMs; 
                
                static const StructDescriptor<NumImplementedFields> &mDescriptor;
        };
    }
    
    namespace EnableNetworkCommand {
        constexpr int NumImplementedFields = GetNumImplementedFields(_Schema);
        extern const StructDescriptor<NumImplementedFields> Descriptor;
        
        class Type {
            public:
                chip::ByteSpan NetworkId;
                uint64_t Breadcrumb;
                uint32_t TimeoutMs; 
                
                static const StructDescriptor<NumImplementedFields> &mDescriptor;
        };
    }
}
}
}
}
