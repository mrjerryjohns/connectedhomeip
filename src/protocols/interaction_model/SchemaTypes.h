/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

/**
 *    @file
 *
 */

#pragma once

#include <core/CHIPCore.h>
#include <core/CHIPTLVDebug.hpp>
#include <support/CodeUtils.h>
#include <core/CHIPConfig.h>
#include <support/BitFlags.h>

namespace chip {
namespace IM {

typedef uint32_t FieldId_t;

constexpr FieldId_t InvalidFieldId() {return 0xffffffff;}
constexpr FieldId_t FieldId(uint16_t vendorId, uint16_t scopedId) { 
    return (((uint32_t)vendorId << 16) | (scopedId & 0xffff)); 
}

typedef uint32_t ClusterId_t;

enum class Type: uint8_t
{
    TYPE_UINT8 = (1 << 0),
    TYPE_UINT32 = (1 << 1),
    TYPE_ARRAY = (1 << 2),
    TYPE_STRUCT = (1 << 3)
};

enum QualityMasks {
    kNone = 0,
    kOptional = (1 << 0),
    kNullable = (1 << 1),
};

struct FieldDescriptor {
    FieldId_t FieldId;
    BitFlags<Type> Type;
    uint32_t Qualities;
    uint32_t Offset;
    uint32_t TypeSize;
    chip::Span<const FieldDescriptor> StructDef;
};

struct TypeOffsetInfo {
    uint32_t Offset;
    uint32_t TypeSize;
};

struct _FieldDescriptor {
    FieldId_t FieldId;
    BitFlags<Type> Type;
    uint32_t Qualities;
    uint64_t FieldGenTag;
    chip::Span<const _FieldDescriptor> FieldList;
};

template <size_t N>
struct BaseType {
    std::array<uint32_t,N> mOffsets;
};

template <size_t N>
struct StructDescriptor {
    std::array<FieldDescriptor, N> FieldList;
};

CHIP_ERROR EncodeSchemaElement(chip::Span<const FieldDescriptor> pDescriptor, void *buf, uint64_t tag, TLV::TLVWriter &writer, FieldId_t field = InvalidFieldId(), bool inArray = false);

#if 0
CHIP_ERROR EncodeSchemaElement(chip::Span<const FieldDescriptor> pDescriptor, void *buf, uint64_t tag, TLV::TLVWriter &writer, FieldId_t field = InvalidFieldId(), bool inArray = false);
#endif

template <typename GenType_t>
CHIP_ERROR EncodeSchemaElement(GenType_t &v, TLV::TLVWriter &writer, uint64_t tag, FieldId_t field = InvalidFieldId())
{
    CHIP_ERROR err = EncodeSchemaElement({v.mDescriptor.FieldList.data(), v.mDescriptor.FieldList.size()}, &v, tag, writer, field);    
    SuccessOrExit(err);

    err = writer.Finalize(); 

exit:
    return err;
}

#if 0
template <typename GenType_t>
CHIP_ERROR DecodeSchemaElement(GenType_t &v, TLV::TLVWriter &writer, FieldId_t field = InvalidFieldId())
{
    SerializeSchemaElement(v.mDescriptor.FieldList, writer, field);    
}
#endif

consteval bool IsImplemented(uint64_t FieldId)
{
#ifdef ImplementedFields
    for (size_t i = 0; i < ImplementedFields.size(); i++) {
        if (ImplementedFields[i] == FieldId) {
            return true;
        }
    }
#endif

    return true;
}

template <size_t N>
consteval int GetNumImplementedFields(const _FieldDescriptor (&structDescriptor)[N])
{
    int count = 0;

    for (size_t i = 0; i < N; i++) {
        if (structDescriptor[i].Qualities & kOptional) {
            count += !!IsImplemented(structDescriptor[i].FieldGenTag);
        }
        else {
            count++;
        }
    }

    return count;
}

template <size_t N, size_t M, class ...Args>
consteval std::array<FieldDescriptor, N> PopulateFieldDescriptors(const _FieldDescriptor (&schema)[M], 
                                                                  std::array<TypeOffsetInfo,N> offsets, const Args& ...args) {
    int index = 0;
    int structIndex = 0;

    std::array<FieldDescriptor, N> r = {};
    std::array<chip::Span<const FieldDescriptor>, sizeof...(args)> structArgs = {{ args... }};

    for (size_t i = 0; i < M; i++) {
        if (((schema[i].Qualities & kOptional) && IsImplemented(schema[i].FieldGenTag)) ||
            !(schema[i].Qualities & kOptional)) {
            r[index].Offset = offsets[index].Offset;
            r[index].TypeSize = offsets[index].TypeSize;
            r[index].FieldId = schema[i].FieldId;
            r[index].Type = schema[i].Type;
            r[index].Qualities = schema[i].Qualities;

            if (schema[i].Type.Has(Type::TYPE_STRUCT)) {
                r[index].StructDef = structArgs[structIndex++];
            }

            index++;
        }
    }

    return r;
}

} // namespace app
} // namespace chip
