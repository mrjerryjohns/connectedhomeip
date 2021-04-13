#include "TestCluster-Gen.h"

namespace chip::IM::Cluster::TestCluster {
    namespace StructA {
        constexpr std::array<TypeOffsetInfo, NumImplementedFields> Offsets = Type::InitializeOffsets();
        
        const StructDescriptor<NumImplementedFields> Descriptor = {
            .FieldList = PopulateFieldDescriptors<NumImplementedFields, std::size(_Schema)>(_Schema, Offsets)
        };

        const StructDescriptor<NumImplementedFields>& Type::mDescriptor = Descriptor;
    }

    namespace StructB {
        constexpr std::array<TypeOffsetInfo, NumImplementedFields> Offsets = Type::InitializeOffsets();
        
        const StructDescriptor<NumImplementedFields> Descriptor = {
            .FieldList = PopulateFieldDescriptors<NumImplementedFields, std::size(_Schema)>(_Schema, Offsets,
                chip::Span<const FieldDescriptor>({StructA::Descriptor.FieldList.data(), StructA::Descriptor.FieldList.size()}))
        };

        const StructDescriptor<NumImplementedFields>& Type::mDescriptor = Descriptor;
    }
    

    namespace Attributes {
        constexpr std::array<TypeOffsetInfo, NumImplementedFields> Offsets = Type::InitializeOffsets();

        const StructDescriptor<NumImplementedFields> Descriptor = {
            .FieldList = PopulateFieldDescriptors<NumImplementedFields, std::size(_Schema)>(_Schema, Offsets,
                chip::Span<const FieldDescriptor>({StructB::Descriptor.FieldList.data(), StructB::Descriptor.FieldList.size()})
            )
        };

        const StructDescriptor<NumImplementedFields>& Type::mDescriptor = Descriptor;
    }
}
