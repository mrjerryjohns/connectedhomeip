#pragma once

#include <type_traits>
#include <array>
#include <core/PeerId.h>
#include <basic-types.h>

namespace chip {
namespace app {
namespace Cluster {
namespace OperationalCredentialCluster {
    const chip::ClusterId kClusterId = 0x003E;

    namespace OpCsrRequest {
        enum FieldID {
            kFieldIdCsrNonce = 0
        };
    }

    namespace OpCsrResponse {
        enum FieldID {
            kFieldIdCsr = 0,
            kFieldIdCsrNonce = 1,
            kFieldIdVendorReserved1 = 2,
            kFieldIdVendorReserved2 = 3,
            kFieldIdVendorReserved3 = 4,
            kFieldIdSignature = 5
        };
    }

    namespace AddOpCert {
        enum FieldId {
            kFieldIdNoc = 0,
            kFieldIdIpkValue = 1,
            kFieldIdCaseAdminNode = 2,
            kFieldIdAdminVendorId = 3
        };
    }

    namespace OpCertResponse {
        enum FieldId {
            kFieldIdStatusCode = 0,
            kFieldIdFabricIndex = 1,
            kFieldIdDebugText = 2
        };
    }
}
}
}
}
