#pragma once

#include <array>
#include <type_traits>
#include "TestCluster.h"

namespace chip {
namespace app {
namespace Cluster { 
namespace TestCluster {
    extern ClusterDescriptor ClusterDescriptor;

    namespace StructA {
        constexpr int NumImplementedFields = GetNumImplementedFields(_Schema);
        extern const StructDescriptor<NumImplementedFields> Descriptor;
        
        class Type {
            public:
                Type() : x(2) {}
                uint8_t x;
                uint8_t y;
                chip::ByteSpan l = chip::ByteSpan{{0, 1, 2, 3}};
                chip::Span<char> m;
                
                static const StructDescriptor<NumImplementedFields> &mDescriptor;
        };
    }
    
    namespace StructB {
        constexpr int NumImplementedFields = GetNumImplementedFields(_Schema);
        extern const StructDescriptor<NumImplementedFields> Descriptor;
        
        class Type {
            public:
                uint8_t x;
                uint8_t y;
                StructA::Type z;

                static const StructDescriptor<NumImplementedFields> &mDescriptor;
        };
    }

    namespace StructC {
        constexpr int NumImplementedFields = GetNumImplementedFields(_Schema);
        extern const StructDescriptor<NumImplementedFields> Descriptor;
        
        class Type {
            public:
                struct empty {};

            public:
                uint8_t a;
                uint8_t b;
                StructA::Type c;
                chip::Span<uint8_t> d;
                chip::Span<StructA::Type> e;

                static const StructDescriptor<NumImplementedFields> &mDescriptor;
        };
    }

    namespace CommandA {
        constexpr int NumImplementedFields = GetNumImplementedFields(_Schema);
        extern const StructDescriptor<NumImplementedFields> Descriptor;
        
        class Type {
            public:
                struct empty {};

            public:
                ClusterId_t GetClusterId() { return kClusterId; }
                uint16_t GetCommandId() { return kCommandAId; }

                uint8_t a;
                uint8_t b;
                StructA::Type c;
                chip::Span<uint8_t> d;

                static const StructDescriptor<NumImplementedFields> &mDescriptor;
        };
    }
    
    namespace CommandB {
        constexpr int NumImplementedFields = GetNumImplementedFields(_Schema);
        extern const StructDescriptor<NumImplementedFields> Descriptor;
        
        class Type {
            public:
                ClusterId_t GetClusterId() { return kClusterId; }
                uint16_t GetCommandId() { return kCommandBId; }
                
                struct empty {};

            public:
                uint8_t a;
                uint8_t b;
                StructA::Type c;
                chip::Span<uint8_t> d;
                chip::Span<StructA::Type> e;

                static const StructDescriptor<NumImplementedFields> &mDescriptor;
        };
    }

    namespace Attributes {
        constexpr int NumImplementedFields = GetNumImplementedFields(_Schema);
        extern const StructDescriptor<NumImplementedFields> Descriptor;
        
        class Type {
            public:
                struct empty {};

                typedef typename std::conditional<IsImplemented(FieldC), uint32_t, empty>::type FieldCType;
                typedef typename std::conditional<IsImplemented(FieldD), uint8_t, empty>::type FieldDType;

                void SetA(uint8_t v) { a = v; }
                void SetC(FieldCType v) { c = v; }

            public:
                uint8_t a;
                uint8_t b;
                FieldCType c;
                FieldDType d;
                chip::Span<uint8_t> e;
                StructB::Type f;

                static const StructDescriptor<NumImplementedFields> &mDescriptor;
        };
    }
}
}
}
}
