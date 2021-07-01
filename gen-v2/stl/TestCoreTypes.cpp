#include "TestCluster-Gen.h"

namespace chip::app::Cluster::CoreTypes {
    namespace Time {
        constexpr std::array<TypeOffsetInfo,NumImplementedFields> InitializeOffsets() {
            std::array<TypeOffsetInfo,NumImplementedFields> r = {};

            uint32_t i = 0;

            r[i++] = {offsetof(class Type, secs), sizeof(Type::secs)};
            r[i++] = {offsetof(class Type, nano_secs), sizeof(Type::nano_secs)};

            return r;
        }

        constexpr std::array<TypeOffsetInfo, NumImplementedFields> Offsets = InitializeOffsets();
        
        const StructDescriptor<NumImplementedFields> Descriptor = {
            .FieldList = PopulateFieldDescriptors<NumImplementedFields, std::size(_Schema)>(_Schema, Offsets)
        };

        const StructDescriptor<NumImplementedFields>& Type::mDescriptor = Descriptor;
    }

}
