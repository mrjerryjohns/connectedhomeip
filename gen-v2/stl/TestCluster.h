#pragma once

#include <type_traits>
#include <array>
#include <SchemaTypes.h>

namespace chip {
namespace app {
namespace Cluster { 
namespace TestCluster {
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
    }
    
    namespace StructB {
        enum FieldId {
            kFieldIdX = 0,
            kFieldIdY = 1,
            kFieldIdZ = 2
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
    }

    namespace CommandA {
        enum FieldId {
            kFieldIdA = 0,
            kFieldIdB = 1,
            kFieldIdC = 2,
            kFieldIdD = 3,
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
    }
}
}
}
}
