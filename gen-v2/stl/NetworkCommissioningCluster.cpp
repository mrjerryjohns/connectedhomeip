#include "NetworkCommissioningCluster-Gen.h"

namespace chip::app::Cluster::NetworkCommissioningCluster {
    struct ClusterDescriptor ClusterDescriptor = {
        .ClusterId = kClusterId
    };
   
    namespace AddThreadNetworkCommand {
        constexpr std::array<TypeOffsetInfo,NumImplementedFields> InitializeOffsets() {
            std::array<TypeOffsetInfo,NumImplementedFields> r = {};

            uint32_t i = 0;

            r[i++] = {offsetof(class Type::Data, OperationalDataSet), sizeof(uint8_t)};
            r[i++] = {offsetof(class Type::Data, Breadcrumb), sizeof(Type::Data::Breadcrumb)};
            r[i++] = {offsetof(class Type::Data, TimeoutMs), sizeof(Type::Data::TimeoutMs)};

            return r;
        }

        constexpr std::array<TypeOffsetInfo, NumImplementedFields> Offsets = InitializeOffsets();
        
        const StructDescriptor<NumImplementedFields> Descriptor = {
            .FieldList = PopulateFieldDescriptors<NumImplementedFields, std::size(_Schema)>(_Schema, Offsets)
        };

        const StructDescriptor<NumImplementedFields>& Type::mDescriptor = Descriptor;
    }
   
#if 0 
    namespace SpecialCommand {
        constexpr std::array<TypeOffsetInfo,NumImplementedFields> InitializeOffsets() {
            std::array<TypeOffsetInfo,NumImplementedFields> r = {};

            uint32_t i = 0;

            r[i++] = {offsetof(class Type::Data, OperationalDataSet), sizeof(uint8_t)};
            r[i++] = {offsetof(class Type::Data, Breadcrumb), sizeof(Type::Data::Breadcrumb)};
            r[i++] = {offsetof(class Type::Data, TimeoutMs), sizeof(Type::Data::TimeoutMs)};

            return r;
        }

        constexpr std::array<TypeOffsetInfo, NumImplementedFields> Offsets = InitializeOffsets();
        
        const StructDescriptor<NumImplementedFields> Descriptor = {
            .FieldList = PopulateFieldDescriptors<NumImplementedFields, std::size(_Schema)>(_Schema, Offsets)
        };

        const StructDescriptor<NumImplementedFields>& Type::mDescriptor = Descriptor;
    }
#endif

    namespace AddWifiNetworkCommand {
        constexpr std::array<TypeOffsetInfo,NumImplementedFields> InitializeOffsets() {
            std::array<TypeOffsetInfo,NumImplementedFields> r = {};

            uint32_t i = 0;

            r[i++] = {offsetof(class Type, Ssid), sizeof(uint8_t)};
            r[i++] = {offsetof(class Type, Credentials), sizeof(uint8_t)};
            r[i++] = {offsetof(class Type, Breadcrumb), sizeof(Type::Breadcrumb)};
            r[i++] = {offsetof(class Type, TimeoutMs), sizeof(Type::TimeoutMs)};

            return r;
        }

        constexpr std::array<TypeOffsetInfo, NumImplementedFields> Offsets = InitializeOffsets();
        
        const StructDescriptor<NumImplementedFields> Descriptor = {
            .FieldList = PopulateFieldDescriptors<NumImplementedFields, std::size(_Schema)>(_Schema, Offsets)
        };

        const StructDescriptor<NumImplementedFields>& Type::mDescriptor = Descriptor;
    }

    namespace EnableNetworkCommand {
        constexpr std::array<TypeOffsetInfo,NumImplementedFields> InitializeOffsets() {
            std::array<TypeOffsetInfo,NumImplementedFields> r = {};

            uint32_t i = 0;

            r[i++] = {offsetof(class Type, NetworkId), sizeof(uint8_t)};
            r[i++] = {offsetof(class Type, Breadcrumb), sizeof(Type::Breadcrumb)};
            r[i++] = {offsetof(class Type, TimeoutMs), sizeof(Type::TimeoutMs)};

            return r;
        }

        constexpr std::array<TypeOffsetInfo, NumImplementedFields> Offsets = InitializeOffsets();
        
        const StructDescriptor<NumImplementedFields> Descriptor = {
            .FieldList = PopulateFieldDescriptors<NumImplementedFields, std::size(_Schema)>(_Schema, Offsets)
        };

        const StructDescriptor<NumImplementedFields>& Type::mDescriptor = Descriptor;
    }
}
