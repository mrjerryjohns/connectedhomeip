#pragma once

#include <array>
#include <type_traits>
#include "TestCoreTypes.h"

namespace chip::app::Cluster::CoreTypes {
    namespace Time {
        constexpr int NumImplementedFields = GetNumImplementedFields(_Schema);
        extern const StructDescriptor<NumImplementedFields> Descriptor;
        
        class Type {
            public:
                struct empty {};

            public:
                uint8_t secs;
                uint8_t nano_secs;
                
                static const StructDescriptor<NumImplementedFields> &mDescriptor;
        };
    }
}
