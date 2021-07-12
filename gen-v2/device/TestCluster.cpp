#include "TestCluster-Gen.h"
namespace chip {
namespace app {
namespace Cluster { 
namespace TestCluster {
    namespace StructA {
        constexpr std::array<TypeOffsetInfo,NumImplementedFields> InitializeOffsets() {
            std::array<TypeOffsetInfo,NumImplementedFields> r = {};

            uint32_t i = 0;

            r[i++] = {offsetof(class Type, x), sizeof(Type::x)};
            r[i++] = {offsetof(class Type, y), sizeof(Type::y)};
            r[i++] = {offsetof(class Type, l), sizeof(uint8_t)};
            r[i++] = {offsetof(class Type, m), sizeof(char)};

            return r;
        }

        constexpr std::array<TypeOffsetInfo, NumImplementedFields> Offsets = InitializeOffsets();
        
        const StructDescriptor<NumImplementedFields> Descriptor = {
            .FieldList = PopulateFieldDescriptors<NumImplementedFields, std::size(_Schema)>(_Schema, Offsets)
        };

        const StructDescriptor<NumImplementedFields>& Type::mDescriptor = Descriptor;
    }

    namespace StructB {
        constexpr std::array<TypeOffsetInfo,NumImplementedFields> InitializeOffsets() {
            std::array<TypeOffsetInfo,NumImplementedFields> r = {};

            uint32_t i = 0;

            r[i++] = {offsetof(class Type, x), sizeof(Type::x)};
            r[i++] = {offsetof(class Type, y), sizeof(Type::y)};
            r[i++] = {offsetof(class Type, z), sizeof(Type::z)};

            return r;
        }
        
        constexpr std::array<TypeOffsetInfo, NumImplementedFields> Offsets = InitializeOffsets();
        
        const StructDescriptor<NumImplementedFields> Descriptor = {
            .FieldList = PopulateFieldDescriptors<NumImplementedFields, std::size(_Schema)>(_Schema, Offsets,
                chip::Span<const FieldDescriptor>({StructA::Descriptor.FieldList.data(), StructA::Descriptor.FieldList.size()}))
        };

        const StructDescriptor<NumImplementedFields>& Type::mDescriptor = Descriptor;
    }
    
    namespace StructC {
        constexpr std::array<TypeOffsetInfo,NumImplementedFields> InitializeOffsets() {
            std::array<TypeOffsetInfo,NumImplementedFields> r = {};

            uint32_t i = 0;

            r[i++] = {offsetof(class Type, a), sizeof(Type::a)};
            r[i++] = {offsetof(class Type, b), sizeof(Type::b)};
            r[i++] = {offsetof(class Type, c), sizeof(Type::c)};
            r[i++] = {offsetof(class Type, d), sizeof(uint8_t)};
            r[i++] = {offsetof(class Type, e), sizeof(StructA::Type)};

            return r;
        }
        
        constexpr std::array<TypeOffsetInfo, NumImplementedFields> Offsets = InitializeOffsets();
        
        const StructDescriptor<NumImplementedFields> Descriptor = {
            .FieldList = PopulateFieldDescriptors<NumImplementedFields, std::size(_Schema)>(_Schema, Offsets,
                chip::Span<const FieldDescriptor>({StructA::Descriptor.FieldList.data(), StructA::Descriptor.FieldList.size()}),
                chip::Span<const FieldDescriptor>({StructA::Descriptor.FieldList.data(), StructA::Descriptor.FieldList.size()}))
        };

        const StructDescriptor<NumImplementedFields>& Type::mDescriptor = Descriptor;
    }

    namespace CommandA {
        constexpr std::array<TypeOffsetInfo,NumImplementedFields> InitializeOffsets() {
            std::array<TypeOffsetInfo,NumImplementedFields> r = {};

            uint32_t i = 0;

            r[i++] = {offsetof(class Type, a), sizeof(Type::a)};
            r[i++] = {offsetof(class Type, b), sizeof(Type::b)};
            r[i++] = {offsetof(class Type, c), sizeof(Type::c)};
            r[i++] = {offsetof(class Type, d), sizeof(uint8_t)};

            return r;
        }
        
        constexpr std::array<TypeOffsetInfo, NumImplementedFields> Offsets = InitializeOffsets();
        
        const StructDescriptor<NumImplementedFields> Descriptor = {
            .FieldList = PopulateFieldDescriptors<NumImplementedFields, std::size(_Schema)>(_Schema, Offsets,
                chip::Span<const FieldDescriptor>({StructA::Descriptor.FieldList.data(), StructA::Descriptor.FieldList.size()}))
        };

        const StructDescriptor<NumImplementedFields>& Type::mDescriptor = Descriptor;
    }
   
    namespace CommandB {
        constexpr std::array<TypeOffsetInfo,NumImplementedFields> InitializeOffsets() {
            std::array<TypeOffsetInfo,NumImplementedFields> r = {};

            uint32_t i = 0;

            r[i++] = {offsetof(class Type, a), sizeof(Type::a)};
            r[i++] = {offsetof(class Type, b), sizeof(Type::b)};
            r[i++] = {offsetof(class Type, c), sizeof(Type::c)};
            r[i++] = {offsetof(class Type, d), sizeof(uint8_t)};
            r[i++] = {offsetof(class Type, e), sizeof(StructA::Type)};

            return r;
        }
        
        constexpr std::array<TypeOffsetInfo, NumImplementedFields> Offsets = InitializeOffsets();
        
        const StructDescriptor<NumImplementedFields> Descriptor = {
            .FieldList = PopulateFieldDescriptors<NumImplementedFields, std::size(_Schema)>(_Schema, Offsets,
                chip::Span<const FieldDescriptor>({StructA::Descriptor.FieldList.data(), StructA::Descriptor.FieldList.size()}),
                chip::Span<const FieldDescriptor>({StructA::Descriptor.FieldList.data(), StructA::Descriptor.FieldList.size()}))
        };

        const StructDescriptor<NumImplementedFields>& Type::mDescriptor = Descriptor;
    }

    namespace Attributes {
        constexpr std::array<TypeOffsetInfo,NumImplementedFields> InitializeOffsets() {
            std::array<TypeOffsetInfo,NumImplementedFields> r = {};

            uint32_t i = 0;

            r[i++] = {offsetof(class Type, a), sizeof(Type::a)};
            r[i++] = {offsetof(class Type, b), sizeof(Type::b)};

            if (IsImplemented(FieldC)) {
                r[i++] = {offsetof(class Type, c), sizeof(Type::c)};
            }

            if (IsImplemented(FieldD)) {
                r[i++] = {offsetof(class Type, d), sizeof(Type::d)};
            }

            if (IsImplemented(FieldE)) {
                r[i++] = {offsetof(class Type, e), sizeof(uint8_t)};
            }

            r[i++] = {offsetof(class Type, f), sizeof(Type::f)};

            return r;
        }
        
        constexpr std::array<TypeOffsetInfo, NumImplementedFields> Offsets = InitializeOffsets();

        const StructDescriptor<NumImplementedFields> Descriptor = {
            .FieldList = PopulateFieldDescriptors<NumImplementedFields, std::size(_Schema)>(_Schema, Offsets,
                chip::Span<const FieldDescriptor>({StructB::Descriptor.FieldList.data(), StructB::Descriptor.FieldList.size()})
            )
        };

        const StructDescriptor<NumImplementedFields>& Type::mDescriptor = Descriptor;
    }
}
}
}
}
