/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2018 Nest Labs, Inc.
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
 *          Provides an implementation of the PlatformManager object.
 */

#pragma once

#include <dispatch/dispatch.h>
#include <platform/internal/GenericPlatformManagerImpl.h>
#include <pthread.h>

static constexpr const char * const CHIP_CONTROLLER_QUEUE = "com.zigbee.chip.framework.controller.workqueue";

namespace chip {
namespace DeviceLayer {

/**
 * Concrete implementation of the PlatformManager singleton object for Darwin platforms.
 */
class PlatformManagerImpl final : public PlatformManager, public Internal::GenericPlatformManagerImpl<PlatformManagerImpl>
{
    // Allow the PlatformManager interface class to delegate method calls to
    // the implementation methods provided by this class.
    friend PlatformManager;

public:
    // ===== Platform-specific members that may be accessed directly by the application.
    dispatch_queue_t GetWorkQueue() { return mWorkQueue; }

private:
    // ===== Methods that implement the PlatformManager abstract interface.
    CHIP_ERROR _InitChipStack();
    CHIP_ERROR _Shutdown();

    CHIP_ERROR _StartChipTimer(int64_t aMilliseconds) { return CHIP_ERROR_NOT_IMPLEMENTED; };
    CHIP_ERROR _StartEventLoopTask();
    CHIP_ERROR _StopEventLoopTask();
    void _RunEventLoop();
    void _LockChipStack() { pthread_mutex_lock(&mStackLock); }
    bool _TryLockChipStack() { return false; };
    void _UnlockChipStack() { pthread_mutex_unlock(&mStackLock); }
    void _PostEvent(const ChipDeviceEvent * event);

#if defined(CHIP_STACK_LOCK_TRACKING_ENABLED)
    bool _IsChipStackLockedByCurrentThread() const { return false; };
#endif

    // ===== Members for internal use by the following friends.

    friend PlatformManager & PlatformMgr(void);
    friend PlatformManagerImpl & PlatformMgrImpl(void);
    friend class Internal::BLEManagerImpl;

    static PlatformManagerImpl sInstance;

    dispatch_queue_t mWorkQueue = nullptr;
    pthread_mutex_t mStackLock;
    pthread_mutex_t mStateLock;
    pthread_cond_t mQueueCond;

    __block bool mQueueIsActive = false;

    inline ImplClass * Impl() { return static_cast<PlatformManagerImpl *>(this); }
};

/**
 * Returns the public interface of the PlatformManager singleton object.
 *
 * chip applications should use this to access features of the PlatformManager object
 * that are common to all platforms.
 */
inline PlatformManager & PlatformMgr(void)
{
    return PlatformManagerImpl::sInstance;
}

/**
 * Returns the platform-specific implementation of the PlatformManager singleton object.
 *
 * chip applications can use this to gain access to features of the PlatformManager
 * that are specific to the ESP32 platform.
 */
inline PlatformManagerImpl & PlatformMgrImpl(void)
{
    return PlatformManagerImpl::sInstance;
}

} // namespace DeviceLayer
} // namespace chip
