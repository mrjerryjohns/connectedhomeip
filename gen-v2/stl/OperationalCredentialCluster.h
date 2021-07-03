#pragma once

#include <type_traits>
#include <array>
#include <SchemaTypes.h>
#include <core/PeerId.h>

namespace chip::app::Cluster::OperationalCredentialCluster {
    const ClusterId_t kClusterId = 0x003E;

    namespace AddOpCert {
        enum FieldId {
            kFieldIdNoc = 0,
            kFieldIdIcaCertificate = 1,
            kFieldIdIpkValue = 2,
            kFieldIdCaseAdminNode = 3,
            kFieldIdAdminVendorId = 4
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
