#pragma once

#include <array>
#include <type_traits>
#include "NetworkCommissioningCluster.h"

namespace chip {
namespace app {
namespace Cluster { 
namespace NetworkCommissioningCluster {
    extern ClusterDescriptor ClusterDescriptor;

    namespace AddThreadNetworkCommand {
        constexpr int NumImplementedFields = GetNumImplementedFields(_Schema);
        extern const StructDescriptor<NumImplementedFields> Descriptor;
        
        class Type {
            public:
                ClusterId_t GetClusterId() { return kClusterId; }
                uint16_t GetCommandId() { return kAddThreadNetworkRequestCommandId; }
                
                chip::ByteSpan OperationalDataSet;
                uint64_t Breadcrumb;
                uint32_t TimeoutMs; 
                
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
