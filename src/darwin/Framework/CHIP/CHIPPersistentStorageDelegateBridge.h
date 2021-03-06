/**
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#import "CHIPPersistentStorageDelegate.h"
#import <Foundation/Foundation.h>

#include <controller/CHIPPersistentStorageDelegate.h>

NS_ASSUME_NONNULL_BEGIN

class CHIPPersistentStorageDelegateBridge : public chip::DeviceController::PersistentStorageDelegate
{
public:
    CHIPPersistentStorageDelegateBridge();
    ~CHIPPersistentStorageDelegateBridge();

    void setFrameworkDelegate(id<CHIPPersistentStorageDelegate> delegate, dispatch_queue_t queue);

    void SetDelegate(chip::DeviceController::PersistentStorageResultDelegate * delegate);

    void GetKeyValue(const char * key) override;

    void SetKeyValue(const char * key, const char * value) override;

    void DeleteKeyValue(const char * key) override;

private:
    id<CHIPPersistentStorageDelegate> mDelegate;
    dispatch_queue_t mQueue;

    chip::DeviceController::PersistentStorageResultDelegate * mCallback;
    SendKeyValue mCompletionHandler;
    SendStatus mStatusHandler;
    NSUserDefaults * mDefaultPersistentStorage;
    dispatch_queue_t mDefaultCallbackQueue;
};

NS_ASSUME_NONNULL_END
