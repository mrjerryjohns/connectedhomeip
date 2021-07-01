#pragma once

#include <array>
#include <type_traits>
#include "TestCluster.h"
#include <vector>
#include <string>

namespace chip::app::Cluster::TestCluster {
    extern ClusterDescriptor ClusterDescriptor;
    constexpr ClusterId_t kClusterId = 0x000000001;

    namespace StructA {
        class Type : public ISerializable {
            public:
                uint8_t x;
                uint8_t y;
                std::vector<uint8_t> l;
                std::string m;

            public:
                CHIP_ERROR Encode(TLV::TLVWriter &writer, uint64_t tag) final; 
                CHIP_ERROR Decode(TLV::TLVReader &reader) final;
        };
    }
    
    namespace StructB {
        class Type : public ISerializable {
            public:
                uint8_t x;
                uint8_t y;
                StructA::Type z;

            public:
                CHIP_ERROR Encode(TLV::TLVWriter &writer, uint64_t tag) final; 
                CHIP_ERROR Decode(TLV::TLVReader &reader) final;
        };
    }

    namespace StructC {
        class Type : public ISerializable {
            public:
                struct empty {};

            public:
                uint8_t a;
                uint8_t b;
                StructA::Type c;
                std::vector<uint8_t> d;
                std::vector<StructA::Type> e;

                CHIP_ERROR Encode(TLV::TLVWriter &writer, uint64_t tag) final; 
                CHIP_ERROR Decode(TLV::TLVReader &reader) final;
        };
    }

#if 0
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

    class ClientCommandsInterface : public InvokeInitiator::CommandHandler {
        public:
            void OnCommandARequest(CommandA::Type &type, InvokeInitiator &initiator) = 0;


    };

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
#endif
}
