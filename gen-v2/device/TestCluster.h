#pragma once

#include <type_traits>
#include <SchemaTypes.h>

namespace chip::IM::Cluster::TestCluster {
    namespace StructA {
        enum FieldId {
            kFieldIdJ = 0,
            kFieldIdK = 1
        };

        constexpr uint64_t FieldJ = (0x00000001ULL << 32) | 0x0b;
        constexpr uint64_t FieldK = (0x00000001ULL << 32) | 0x0c;

        constexpr _FieldDescriptor _Schema[] = {
            {kFieldIdJ, Type::TYPE_UINT8, kNone,                  FieldJ, {}},
            {kFieldIdK, Type::TYPE_UINT8, kNullable,              FieldK, {}},
        };
    }
    
    namespace StructB {
        enum FieldId {
            kFieldIdX = 0,
            kFieldIdY = 1,
            kFieldIdZ = 2
        };

        constexpr uint64_t FieldX = (0x00000001ULL << 32) | 0x08;
        constexpr uint64_t FieldY = (0x00000001ULL << 32) | 0x09;
        constexpr uint64_t FieldZ = (0x00000001ULL << 32) | 0x0a;

        constexpr _FieldDescriptor _Schema[] = {
            {kFieldIdX, Type::TYPE_UINT8, kNone,                  FieldX, {}},
            {kFieldIdY, Type::TYPE_UINT8, kNullable,              FieldY, {}},
            {kFieldIdZ, Type::TYPE_STRUCT, 0,                     FieldZ, {StructA::_Schema, std::size(StructA::_Schema)}},
        };
    }

    namespace Attributes {
        enum FieldId {
            kFieldIdA = 0,
            kFieldIdB = 1,
            kFieldIdC = 2,
            kFieldIdD = 3,
            kFieldIdE = 4,
            kFieldIdF = 5
        };

        constexpr uint64_t FieldA = (0x00000001ULL << 32) | 0x01;
        constexpr uint64_t FieldB = (0x00000001ULL << 32) | 0x02;
        constexpr uint64_t FieldC = (0x00000001ULL << 32) | 0x04;
        constexpr uint64_t FieldD = (0x00000001ULL << 32) | 0x05;
        constexpr uint64_t FieldE = (0x00000001ULL << 32) | 0x06;
        constexpr uint64_t FieldF = (0x00000001ULL << 32) | 0x07;
        
        constexpr _FieldDescriptor _Schema[] = {
            {kFieldIdA, Type::TYPE_UINT8,           kNone,                  FieldA,     {}},
            {kFieldIdB, Type::TYPE_UINT8,           kNullable,              FieldB,     {}},
            {kFieldIdC, Type::TYPE_UINT32,          kOptional | kNullable,  FieldC,     {}},
            {kFieldIdD, Type::TYPE_UINT8,           kOptional | kNullable,  FieldD,     {}},
            {kFieldIdE, BitFlags(Type::TYPE_ARRAY).Set(Type::TYPE_UINT8), kNone,                  FieldF,     {}},
            {kFieldIdF, Type::TYPE_STRUCT,          kNone,                  FieldE,     {StructB::_Schema, std::size(StructB::_Schema)}},
        };
    }
}
