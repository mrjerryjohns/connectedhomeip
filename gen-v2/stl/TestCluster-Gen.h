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
namespace TestCluster {
    constexpr chip::ClusterId kClusterId = 0x000000001;

    enum CommandId {
        kCommandAId = 1,
        kCommandBId = 2
    };
    
    namespace StructA {
        enum FieldId {
            kFieldIdJ = 0,
            kFieldIdK = 1,
            kFieldIdL = 2,
            kFieldIdM = 3
        };
        
        class Type : public IEncodable {
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
        enum FieldId {
            kFieldIdX = 0,
            kFieldIdY = 1,
            kFieldIdZ = 2
        };
        
        class Type : public IEncodable {
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
        enum FieldId {
            kFieldIdA = 0,
            kFieldIdB = 1,
            kFieldIdC = 2,
            kFieldIdD = 3,
            kFieldIdE = 4,
        };
        
        class Type : public IEncodable {
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

    namespace CommandA {
        enum FieldId {
            kFieldIdA = 0,
            kFieldIdB = 1,
            kFieldIdC = 2,
            kFieldIdD = 3,
        };
        
        class Type : public IEncodable {
            public:
                static chip::ClusterId GetClusterId() { return kClusterId; }
                static uint16_t GetCommandId() { return kCommandAId; }

                uint8_t a;
                uint8_t b;
                StructA::Type c;
                std::vector<uint8_t> d;

                CHIP_ERROR Encode(TLV::TLVWriter &writer, uint64_t tag) final; 
                CHIP_ERROR Decode(TLV::TLVReader &reader) final;
        };
    }
    
    namespace CommandB {
        enum FieldId {
            kFieldIdA = 0,
            kFieldIdB = 1,
            kFieldIdC = 2,
            kFieldIdD = 3,
            kFieldIdE = 4,
        };
        
        class Type : public IEncodable {
            public:
                static chip::ClusterId GetClusterId() { return kClusterId; }
                static uint16_t GetCommandId() { return kCommandBId; }
                
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
}
}
}
}
