#pragma once

#include <type_traits>
#include <array>
#include <SchemaTypes.h>
#include <basic-types.h>

namespace chip {
namespace app {
namespace Cluster { 
namespace CoreTypes {
    namespace Time {
        enum FieldId {
            kFieldSecs = 0,
            kFieldNanoSecs = 1
        };

        constexpr uint64_t FieldSecs = (0x00000009ULL << 32) | 0x0b;
        constexpr uint64_t FieldNanoSecs = (0x00000009ULL << 32) | 0x0c;

        constexpr _FieldDescriptor _Schema[] = {
            {kFieldSecs, Type::TYPE_UINT8, kNone,                  FieldSecs, {}},
            {kFieldNanoSecs, Type::TYPE_UINT8, kNullable,              FieldNanoSecs, {}},
        };
    }
}
}
}
}
