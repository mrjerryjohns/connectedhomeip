#include "OperationalCredentialCluster-Gen.h"
#include "OperationalCredentialCluster.h"
#include "core/CHIPTLVTags.h"
#include "core/CHIPTLVTypes.h"

namespace chip::app::Cluster::OperationalCredentialCluster {
    struct ClusterDescriptor ClusterDescriptor = {
        .ClusterId = kClusterId
    };

    namespace AddOpCert {
        CHIP_ERROR Type::Encode(TLV::TLVWriter &writer, uint64_t tag) {
            TLV::TLVType outer;
            ReturnErrorOnFailure(writer.StartContainer(tag, TLV::kTLVType_Structure, outer));
            ReturnErrorOnFailure(writer.PutBytes(TLV::ContextTag(kFieldIdNoc), &noc[0], noc.size()));
            ReturnErrorOnFailure(writer.PutBytes(TLV::ContextTag(kFieldIdIcaCertificate), &icaCertificate[0], icaCertificate.size()));
            ReturnErrorOnFailure(writer.PutBytes(TLV::ContextTag(kFieldIdIpkValue), &ipkValue[0], ipkValue.size()));
            ReturnErrorOnFailure(writer.Put(TLV::ContextTag(kFieldIdCaseAdminNode), caseAdminNode));
            ReturnErrorOnFailure(writer.Put(TLV::ContextTag(kFieldIdAdminVendorId), adminVendorId));
            ReturnErrorOnFailure(writer.EndContainer(outer));
            ReturnErrorOnFailure(writer.Finalize());
            return CHIP_NO_ERROR;
        }

        CHIP_ERROR Type::Decode(TLV::TLVReader &reader) {
            CHIP_ERROR err;
            TLV::TLVType outer;

            err = reader.EnterContainer(outer);
            ReturnErrorOnFailure(err);

            while ((err = reader.Next()) == CHIP_NO_ERROR) {
                uint64_t tag = reader.GetTag();

                if (tag == TLV::ContextTag(kFieldIdNoc)) {
                   noc.resize(reader.GetLength());
                   ReturnErrorOnFailure(reader.GetBytes(noc.data(), reader.GetLength()));
                }
                else if (tag == TLV::ContextTag(kFieldIdIcaCertificate)) {
                   icaCertificate.resize(reader.GetLength());
                   ReturnErrorOnFailure(reader.GetBytes(icaCertificate.data(), reader.GetLength()));
                }
                else if (tag == TLV::ContextTag(kFieldIdIpkValue)) {
                   ipkValue.resize(reader.GetLength());
                   ReturnErrorOnFailure(reader.GetBytes(ipkValue.data(), reader.GetLength()));
                }
                else if (tag == TLV::ContextTag(kFieldIdCaseAdminNode)) {
                    ReturnErrorOnFailure(reader.Get(caseAdminNode));
                }
                else if (tag == TLV::ContextTag(kFieldIdAdminVendorId)) {
                    ReturnErrorOnFailure(reader.Get(adminVendorId));
                }
            }

            ReturnErrorOnFailure(reader.ExitContainer(outer));
            return CHIP_NO_ERROR;
        }
    }

    namespace OpCertResponse {
        CHIP_ERROR Type::Encode(TLV::TLVWriter &writer, uint64_t tag) {
            TLV::TLVType outer;
            ReturnErrorOnFailure(writer.StartContainer(tag, TLV::kTLVType_Structure, outer));
            ReturnErrorOnFailure(writer.Put(TLV::ContextTag(kFieldIdStatusCode), statusCode));
            ReturnErrorOnFailure(writer.Put(TLV::ContextTag(kFieldIdFabricIndex), fabricIndex));
            ReturnErrorOnFailure(writer.PutString(TLV::ContextTag(kFieldIdDebugText), debugText.c_str(), debugText.length()));
            ReturnErrorOnFailure(writer.EndContainer(outer));
            ReturnErrorOnFailure(writer.Finalize());
            return CHIP_NO_ERROR;
        }

        CHIP_ERROR Type::Decode(TLV::TLVReader &reader) {
            CHIP_ERROR err;
            TLV::TLVType outer;

            err = reader.EnterContainer(outer);
            ReturnErrorOnFailure(err);

            while ((err = reader.Next()) == CHIP_NO_ERROR) {
                uint64_t tag = reader.GetTag();


                if (tag == TLV::ContextTag(kFieldIdStatusCode)) {
                    ReturnErrorOnFailure(reader.Get(statusCode));
                }
                else if (tag == TLV::ContextTag(kFieldIdFabricIndex)) {
                    ReturnErrorOnFailure(reader.Get(fabricIndex));
                }
                else if (tag == TLV::ContextTag(kFieldIdDebugText)) {
                   int len = reader.GetLength() + 1;
                   debugText.resize(len + 1);
                   ReturnErrorOnFailure(reader.GetString(debugText.data(), len + 1));
                   debugText.resize(len - 1);
                }
            }

            ReturnErrorOnFailure(reader.ExitContainer(outer));
            return CHIP_NO_ERROR;
        }
    }
}
