#pragma once

#include <array>
#include <type_traits>
#include "TestCluster.h"

namespace chip::IM::Cluster::TestCluster {
    namespace StructA {
        constexpr int NumImplementedFields = GetNumImplementedFields(_Schema);
        extern const StructDescriptor<NumImplementedFields> Descriptor;
        
        class Type {
            public:
                struct empty {};

                void SetX(int8_t v) { x = v; }
                void SetY(int8_t v) { y = v; }

                int8_t x;
                int8_t y;
               
                static consteval std::array<TypeOffsetInfo,NumImplementedFields> InitializeOffsets() {
                    std::array<TypeOffsetInfo,NumImplementedFields> r = {};

                    int i = 0;

                    r[i++] = {offsetof(Type, x), sizeof(x)};
                    r[i++] = {offsetof(Type, y), sizeof(y)};

                    return r;
                }
                
                static const StructDescriptor<NumImplementedFields> &mDescriptor;
        };
    }
    
    namespace StructB {
        constexpr int NumImplementedFields = GetNumImplementedFields(_Schema);
        extern const StructDescriptor<NumImplementedFields> Descriptor;
        
        class Type {
            public:
                struct empty {};
                typedef typename std::conditional<IsImplemented(FieldZ), StructA::Type, empty>::type FieldZType;

                void SetX(int8_t v) { x = v; }
                void SetY(int8_t v) { y = v; }

                int8_t x;
                int8_t y;
                FieldZType z;

                consteval static std::array<TypeOffsetInfo,NumImplementedFields> InitializeOffsets() {
                    std::array<TypeOffsetInfo,NumImplementedFields> r = {};

                    int i = 0;

                    r[i++] = {offsetof(Type, x), sizeof(x)};
                    r[i++] = {offsetof(Type, y), sizeof(y)};
                    r[i++] = {offsetof(Type, z), sizeof(z)};

                    return r;
                }
                
                static const StructDescriptor<NumImplementedFields> &mDescriptor;
        };
    }

    namespace Attributes {
        constexpr int NumImplementedFields = GetNumImplementedFields(_Schema);
        extern const StructDescriptor<NumImplementedFields> Descriptor;
        
        struct Type {
            public:
                struct empty {};

                typedef typename std::conditional<IsImplemented(FieldC), uint32_t, empty>::type FieldCType;
                typedef typename std::conditional<IsImplemented(FieldD), uint32_t, empty>::type FieldDType;
                typedef typename std::conditional<IsImplemented(FieldE), StructA::Type, empty>::type FieldEType;

                void SetA(int8_t v) { a = v; }
                void SetC(FieldCType v) { c = v; }

                int8_t a;
                int8_t b;

                FieldCType c;
                FieldDType d;

                chip::Span<uint8_t> f;

                [[no_unique_address]] FieldEType e;

                consteval static std::array<TypeOffsetInfo,NumImplementedFields> InitializeOffsets() {
                    std::array<TypeOffsetInfo,NumImplementedFields> r = {};

                    int i = 0;

                    r[i++] = {offsetof(Type, a), sizeof(a)};
                    r[i++] = {offsetof(Type, b), sizeof(b)};

                    if (IsImplemented(FieldC)) {
                        r[i++] = {offsetof(Type, c), sizeof(c)};
                    }

                    if (IsImplemented(FieldD)) {
                        r[i++] = {offsetof(Type, d), sizeof(c)};
                    }

                    if (IsImplemented(FieldE)) {
                        r[i++] = {offsetof(Type, e), sizeof(e)};
                    }

                    r[i++] = {offsetof(Type, f), sizeof(uint8_t)};

                    return r;
                }
                
                static const StructDescriptor<NumImplementedFields> &mDescriptor;
        };
    }
    
}
