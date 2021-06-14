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
 *          Provides an implementation of the PlatformManager object
 *          for Darwin platforms.
 */

#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/PlatformManager.h>

// Include the non-inline definitions for the GenericPlatformManagerImpl<> template,
#include <platform/internal/GenericPlatformManagerImpl.cpp>

namespace chip {
namespace DeviceLayer {

PlatformManagerImpl PlatformManagerImpl::sInstance;

CHIP_ERROR PlatformManagerImpl::_InitChipStack()
{
    CHIP_ERROR err;

    // Initialize the configuration system.
    err = Internal::PosixConfig::Init();
    SuccessOrExit(err);

    pthread_mutex_init(&mStackLock, nullptr);
    pthread_mutex_init(&mStateLock, nullptr);
    pthread_cond_init(&mQueueCond, nullptr);

    //
    // Create the work-queue before we initialize the CHIP stack, since that
    // might result in PostEvent being invoked, so we need to ensure the queue is ready to be postable to.
    //
    if (mWorkQueue == nullptr) {
        mWorkQueue = dispatch_queue_create(CHIP_CONTROLLER_QUEUE, DISPATCH_QUEUE_SERIAL);
    }

    //
    // Prevent actual execution of the queue till a call to either StartEventLoopTask() or RunEventLoop() has been made. 
    // Otherwise, we will end up having race conditions between queue items running, and calls being made to the stack
    // without locks held from outside of the dispatch queue.
    //
    dispatch_suspend(mWorkQueue);

    mQueueIsActive = true;

    // Call _InitChipStack() on the generic implementation base class
    // to finish the initialization process.
    err = Internal::GenericPlatformManagerImpl<PlatformManagerImpl>::_InitChipStack();
    SuccessOrExit(err);

    SystemLayer.SetDispatchQueue(mWorkQueue);

exit:
    return err;
}

CHIP_ERROR PlatformManagerImpl::_StartEventLoopTask()
{
    //
    // There is no separate task that needs to be created. With our already
    // created work queue, just resume the dispatch on it.
    //
    dispatch_resume(mWorkQueue);
    return CHIP_NO_ERROR;
}

CHIP_ERROR PlatformManagerImpl::_StopEventLoopTask()
{
    //
    // GCD provides a mechanism to suspend un-executed work items in the dispatch queue,
    // but doesn't provide a synchronous way to block till a currently executing block has
    // completed. This logic below makes that possible by synchronously dispatching a simple
    // block that sets the mQueueIsActive to false (causing newer tasks that were queued up
    // after to just be dropped on the ground after), and waiting till setting that variable is
    // complete.
    //
  
    dispatch_sync(mWorkQueue, ^{
        mQueueIsActive = false; 
    });

    //
    // Wake up anyone blocked at RunEventLoop
    //
    pthread_cond_signal(&mQueueCond);

    return CHIP_NO_ERROR;
}

void PlatformManagerImpl::_RunEventLoop()
{
    dispatch_resume(mWorkQueue);

    pthread_mutex_lock(&mStateLock);

    //
    // GCD doesn't provide a method that allows us to 'block' on a dispatch queue
    // so we need to resort to pthread mutex + cond var to achieve the same effect.
    //
    while (mQueueIsActive) {
        pthread_cond_wait(&mQueueCond, &mStateLock);
    }

    pthread_mutex_unlock(&mStateLock);
}

CHIP_ERROR PlatformManagerImpl::_Shutdown()
{
    // Call up to the base class _Shutdown() to perform the bulk of the shutdown.
    return System::MapErrorPOSIX(GenericPlatformManagerImpl<ImplClass>::_Shutdown());
}

void PlatformManagerImpl::_PostEvent(const ChipDeviceEvent * event)
{

    const ChipDeviceEvent eventCopy = *event;
    dispatch_async(mWorkQueue, ^{
        //
        // If the queue isn't active, drop this on the floor since the stack is in the process,
        // or already has, shutdown
        //
        if (!mQueueIsActive) {
            return;
        }
       
        pthread_mutex_lock(&mStackLock);
        Impl()->DispatchEvent(&eventCopy);
        pthread_mutex_unlock(&mStackLock);
    });
}

} // namespace DeviceLayer
} // namespace chip
