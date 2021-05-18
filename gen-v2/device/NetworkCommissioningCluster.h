#pragma once

#include <type_traits>
#include <array>
#include <SchemaTypes.h>

namespace chip::app::Cluster::NetworkCommissioningCluster {
    constexpr ClusterId_t kClusterId = 0x0031;

    enum CommandId {
        kScanNetworksRequestCommandId = 0,
        kScanNetworksRespCommandId = 1,
        kAddWifiNetworkRequestCommandId = 2,
        kAddWifiNetworkResponseCommandId = 3,
        kUpdateWifiNetworkRequestCommandId = 4,
        kUpdateWifiNetworkResponseCommandId = 5,
        kAddThreadNetworkRequestCommandId = 6,
        kAddThreadNetworkResponseCommandId = 7,
        kUpdateThreadNetworkRequestCommandId = 8,
        kUpdateThreadNetworkResponseCommandId = 9,
        kRemoveNetworkRequestCommandId = 10,
        kRemoveNetworkResponseCommandId = 11,
        kEnableNetworkRequestCommandId = 12,
        kEnableNetworkResponseCommandId = 13,
        kDisableNetworkRequestCommandId = 14,
        kDisableNetworkResponseCommandId = 15,
        kGetLastNetworkCommissioningResultRequestCommandId = 16
    };

    namespace AddThreadNetworkCommand {
        enum FieldId {
            kOperationalDatasetFieldId = 0,
            kBreadcrumbFieldId = 1,
            kTimeoutMsFieldId = 2
        };

        constexpr _FieldDescriptor _Schema[] = {
            {kOperationalDatasetFieldId,    BitFlags(Type::TYPE_OCTSTR),      kNone,   0,       {}},
            {kBreadcrumbFieldId,            BitFlags(Type::TYPE_UINT64),      kNone,   0,       {}},
            {kTimeoutMsFieldId,             BitFlags(Type::TYPE_UINT32),      kNone,   0,       {}} 
        };
    };

    namespace AddWifiNetworkCommand {
        enum FieldId {
            kSsidFieldId = 0,
            kCredentialsFieldId = 1,
            kBreadcrumbFieldId = 2,
            kTimeoutMsFieldId = 3
        };

        constexpr _FieldDescriptor _Schema[] = {
            {kSsidFieldId,                  BitFlags(Type::TYPE_OCTSTR),      kNone,   0,       {}},
            {kCredentialsFieldId,           BitFlags(Type::TYPE_OCTSTR),      kNone,   0,       {}},
            {kBreadcrumbFieldId,            BitFlags(Type::TYPE_UINT64),      kNone,   0,       {}},
            {kTimeoutMsFieldId,             BitFlags(Type::TYPE_UINT32),      kNone,   0,       {}} 
        };
    };

    namespace DisableNetworkCommand {
        enum FieldId {
            kNetworkId = 0,
            kBreadcrumbFieldId = 1,
            kTimeoutMsFieldId = 2
        };

        constexpr _FieldDescriptor _Schema[] = {
            {kNetworkId,                    BitFlags(Type::TYPE_OCTSTR),      kNone,   0,       {}},
            {kBreadcrumbFieldId,            BitFlags(Type::TYPE_UINT64),      kNone,   0,       {}},
            {kTimeoutMsFieldId,             BitFlags(Type::TYPE_UINT32),      kNone,   0,       {}} 
        };
    };

    namespace EnableNetworkCommand {
        enum FieldId {
            kNetworkId = 0,
            kBreadcrumbFieldId = 1,
            kTimeoutMsFieldId = 2
        };

        constexpr _FieldDescriptor _Schema[] = {
            {kNetworkId,                    BitFlags(Type::TYPE_OCTSTR),      kNone,   0,       {}},
            {kBreadcrumbFieldId,            BitFlags(Type::TYPE_UINT64),      kNone,   0,       {}},
            {kTimeoutMsFieldId,             BitFlags(Type::TYPE_UINT32),      kNone,   0,       {}} 
        };
    };

    namespace RemoveNetworkCommand {
        enum FieldId {
            kNetworkId = 0,
            kBreadcrumbFieldId = 1,
            kTimeoutMsFieldId = 2
        };

        constexpr _FieldDescriptor _Schema[] = {
            {kNetworkId,                    BitFlags(Type::TYPE_OCTSTR),      kNone,   0,       {}},
            {kBreadcrumbFieldId,            BitFlags(Type::TYPE_UINT64),      kNone,   0,       {}},
            {kTimeoutMsFieldId,             BitFlags(Type::TYPE_UINT32),      kNone,   0,       {}} 
        };
    };
    
    
    namespace GetLastNetworkCommissioningResultCommand {
        enum FieldId {
            kTimeoutMsFieldId = 0
        };

        constexpr _FieldDescriptor _Schema[] = {
            {kTimeoutMsFieldId,             BitFlags(Type::TYPE_UINT32),      kNone,   0,       {}} 
        };
    };
}
