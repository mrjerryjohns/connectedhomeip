/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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
 *      This file defines Base class for a CHIP IM Command
 *
 */

#include "SchemaTypes.h"
#include "core/CHIPTLVTags.h"
#include "core/CHIPTLVTypes.h"
#include <core/CHIPTLVDebug.hpp>

struct Test {
};

static_assert(sizeof(chip::Span<uint8_t>) == sizeof(chip::Span<Test>), "Chip::Span cannot be type erasured");

namespace chip {
namespace IM {

CHIP_ERROR EncodeSchemaElement(chip::Span<const FieldDescriptor> pDescriptor, void *buf, uint64_t tag, TLV::TLVWriter &writer, FieldId_t field, bool inArray)
{
    CHIP_ERROR err;
    TLV::TLVType outerContainerType;

    if (field == InvalidFieldId() && !inArray) {
        err = writer.StartContainer(tag, TLV::kTLVType_Structure, outerContainerType);
        SuccessOrExit(err);
    }

    for (auto schemaIter = pDescriptor.data(); schemaIter != (pDescriptor.data() + pDescriptor.size()); schemaIter++) {
        if (field == InvalidFieldId() || schemaIter->FieldId == field) {
            uint64_t fieldTag = inArray ? TLV::AnonymousTag : TLV::ContextTag((uint8_t)schemaIter->FieldId);

            if (schemaIter->Type.Has(Type::TYPE_STRUCT)) {
                err = EncodeSchemaElement(schemaIter->StructDef, (void *)((uintptr_t)(buf) + schemaIter->Offset), 
                                          fieldTag, writer, InvalidFieldId(), false);
                SuccessOrExit(err);
            }
            else if (schemaIter->Type.Has(Type::TYPE_ARRAY)) {
                TLV::TLVType outerContainerType2;
                chip::ByteSpan &p = *(reinterpret_cast<chip::ByteSpan *>((uintptr_t)(buf) + schemaIter->Offset));
                FieldDescriptor tmpDescriptor = *schemaIter;
                chip::Span<const FieldDescriptor> tmpDescriptorList = {&tmpDescriptor, 1};

                tmpDescriptor.Type.Clear(Type::TYPE_ARRAY);
                tmpDescriptor.Offset = 0;

                err = writer.StartContainer(TLV::ContextTag((uint8_t)(schemaIter->FieldId)), TLV::kTLVType_Array, outerContainerType2);
                SuccessOrExit(err);

                for (const uint8_t *ptr = p.data(); ptr < (p.data() + p.size() * schemaIter->TypeSize); ptr += schemaIter->TypeSize) {
                    err = EncodeSchemaElement(tmpDescriptorList, (void *)ptr, 0, writer, InvalidFieldId(), true);
                    SuccessOrExit(err);
                }

                err = writer.EndContainer(outerContainerType2);
                SuccessOrExit(err);
            }
            else if (schemaIter->Type.Has(Type::TYPE_UINT8)) {
                uint8_t *v = (uint8_t *)((uintptr_t)(buf) + schemaIter->Offset);
                err = writer.Put(fieldTag, *v);
                SuccessOrExit(err);
            }
            else if (schemaIter->Type.Has(Type::TYPE_UINT32)) {
                uint32_t *v = (uint32_t *)((uintptr_t)(buf) + schemaIter->Offset);
                err = writer.Put(fieldTag, *v);
                SuccessOrExit(err);
            }
        }
    }

    if (field == InvalidFieldId() && !inArray) {
        err = writer.EndContainer(outerContainerType);
        SuccessOrExit(err);
    }

exit:
    return err;
}


} // namespace app
} // namespace chip
