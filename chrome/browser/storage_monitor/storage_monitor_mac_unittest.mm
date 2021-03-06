// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/storage_monitor/storage_monitor_mac.h"

#include "base/file_util.h"
#include "base/files/scoped_temp_dir.h"
#include "base/mac/foundation_util.h"
#include "base/message_loop.h"
#include "base/run_loop.h"
#include "base/strings/sys_string_conversions.h"
#include "base/utf_string_conversions.h"
#include "chrome/browser/storage_monitor/mock_removable_storage_observer.h"
#include "chrome/browser/storage_monitor/removable_device_constants.h"
#include "chrome/browser/storage_monitor/storage_info.h"
#include "content/public/test/test_browser_thread.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace chrome {

uint64 kTestSize = 1000000ULL;

namespace {

StorageInfo CreateStorageInfo(
    const std::string& device_id,
    const std::string& model_name,
    const base::FilePath& mount_point,
    uint64 size_bytes) {
  return StorageInfo(
      device_id, string16(), mount_point.value(),
      string16(), string16(), UTF8ToUTF16(model_name),
      size_bytes);
}

}  // namespace

class StorageMonitorMacTest : public testing::Test {
 public:
  StorageMonitorMacTest()
      : ui_thread_(content::BrowserThread::UI, &message_loop_),
        file_thread_(content::BrowserThread::FILE, &message_loop_) {
  }

  virtual void SetUp() OVERRIDE {
    monitor_.reset(new StorageMonitorMac);

    mock_storage_observer_.reset(new MockRemovableStorageObserver);
    monitor_->AddObserver(mock_storage_observer_.get());

    unique_id_ = "test_id";
    mount_point_ = base::FilePath("/unused_test_directory");
    device_id_ = StorageInfo::MakeDeviceId(
        StorageInfo::REMOVABLE_MASS_STORAGE_NO_DCIM, unique_id_);
    disk_info_ = CreateStorageInfo(device_id_, "",
                                   mount_point_, kTestSize);
  }

  void UpdateDisk(StorageInfo info, StorageMonitorMac::UpdateType update_type) {
    content::BrowserThread::PostTask(content::BrowserThread::UI, FROM_HERE,
        base::Bind(&StorageMonitorMac::UpdateDisk,
                   base::Unretained(monitor_.get()),
                   "dummy_bsd_name", info, update_type));
    base::RunLoop().RunUntilIdle();
  }

 protected:
  // The message loop and file thread to run tests on.
  MessageLoopForUI message_loop_;
  content::TestBrowserThread ui_thread_;
  content::TestBrowserThread file_thread_;

  scoped_ptr<MockRemovableStorageObserver> mock_storage_observer_;

  // Information about the disk.
  std::string unique_id_;
  base::FilePath mount_point_;
  std::string device_id_;
  StorageInfo disk_info_;

  scoped_ptr<StorageMonitorMac> monitor_;
};

TEST_F(StorageMonitorMacTest, AddRemove) {
  UpdateDisk(disk_info_, StorageMonitorMac::UPDATE_DEVICE_ADDED);

  EXPECT_EQ(1, mock_storage_observer_->attach_calls());
  EXPECT_EQ(0, mock_storage_observer_->detach_calls());
  EXPECT_EQ(device_id_, mock_storage_observer_->last_attached().device_id);
  EXPECT_EQ(string16(), mock_storage_observer_->last_attached().name);
  EXPECT_EQ(mount_point_.value(),
            mock_storage_observer_->last_attached().location);

  UpdateDisk(disk_info_, StorageMonitorMac::UPDATE_DEVICE_REMOVED);

  EXPECT_EQ(1, mock_storage_observer_->attach_calls());
  EXPECT_EQ(1, mock_storage_observer_->detach_calls());
  EXPECT_EQ(device_id_, mock_storage_observer_->last_detached().device_id);
}

TEST_F(StorageMonitorMacTest, UpdateVolumeName) {
  UpdateDisk(disk_info_, StorageMonitorMac::UPDATE_DEVICE_ADDED);

  EXPECT_EQ(1, mock_storage_observer_->attach_calls());
  EXPECT_EQ(0, mock_storage_observer_->detach_calls());
  EXPECT_EQ(device_id_, mock_storage_observer_->last_attached().device_id);
  EXPECT_EQ(string16(), mock_storage_observer_->last_attached().name);
  EXPECT_EQ(kTestSize,
            mock_storage_observer_->last_attached().total_size_in_bytes);
  EXPECT_EQ(mount_point_.value(),
            mock_storage_observer_->last_attached().location);

  StorageInfo info2 = CreateStorageInfo(
      device_id_, "", mount_point_, kTestSize * 2);
  UpdateDisk(info2, StorageMonitorMac::UPDATE_DEVICE_CHANGED);
  message_loop_.RunUntilIdle();

  EXPECT_EQ(1, mock_storage_observer_->detach_calls());
  EXPECT_EQ(device_id_, mock_storage_observer_->last_detached().device_id);
  EXPECT_EQ(2, mock_storage_observer_->attach_calls());
  EXPECT_EQ(device_id_, mock_storage_observer_->last_attached().device_id);
  EXPECT_EQ(string16(), mock_storage_observer_->last_attached().name);
  EXPECT_EQ(kTestSize * 2,
            mock_storage_observer_->last_attached().total_size_in_bytes);
  EXPECT_EQ(mount_point_.value(),
            mock_storage_observer_->last_attached().location);
}

TEST_F(StorageMonitorMacTest, DCIM) {
  base::ScopedTempDir temp_dir;
  ASSERT_TRUE(temp_dir.CreateUniqueTempDir());
  ASSERT_TRUE(file_util::CreateDirectory(
      temp_dir.path().Append(kDCIMDirectoryName)));

  base::FilePath mount_point = temp_dir.path();
  std::string device_id = StorageInfo::MakeDeviceId(
      StorageInfo::REMOVABLE_MASS_STORAGE_WITH_DCIM, unique_id_);
  StorageInfo info = CreateStorageInfo(device_id, "", mount_point, kTestSize);

  UpdateDisk(info, StorageMonitorMac::UPDATE_DEVICE_ADDED);

  EXPECT_EQ(1, mock_storage_observer_->attach_calls());
  EXPECT_EQ(0, mock_storage_observer_->detach_calls());
  EXPECT_EQ(device_id, mock_storage_observer_->last_attached().device_id);
  EXPECT_EQ(string16(), mock_storage_observer_->last_attached().name);
  EXPECT_EQ(mount_point.value(),
            mock_storage_observer_->last_attached().location);
}

TEST_F(StorageMonitorMacTest, GetStorageInfo) {
  UpdateDisk(disk_info_, StorageMonitorMac::UPDATE_DEVICE_ADDED);

  EXPECT_EQ(1, mock_storage_observer_->attach_calls());
  EXPECT_EQ(0, mock_storage_observer_->detach_calls());
  EXPECT_EQ(device_id_, mock_storage_observer_->last_attached().device_id);
  EXPECT_EQ(string16(), mock_storage_observer_->last_attached().name);
  EXPECT_EQ(mount_point_.value(),
            mock_storage_observer_->last_attached().location);

  StorageInfo info;
  EXPECT_TRUE(monitor_->GetStorageInfoForPath(mount_point_.AppendASCII("foo"),
                                              &info));
  EXPECT_EQ(device_id_, info.device_id);
  EXPECT_EQ(string16(), info.name);
  EXPECT_EQ(mount_point_.value(), info.location);
  EXPECT_EQ(kTestSize, info.total_size_in_bytes);

  EXPECT_FALSE(monitor_->GetStorageInfoForPath(
      base::FilePath("/non/matching/path"), &info));
}

// Test that mounting a DMG doesn't send a notification.
TEST_F(StorageMonitorMacTest, DMG) {
  StorageInfo info = CreateStorageInfo(
      device_id_, "Disk Image", mount_point_, kTestSize);
  UpdateDisk(info, StorageMonitorMac::UPDATE_DEVICE_ADDED);
  EXPECT_EQ(0, mock_storage_observer_->attach_calls());
}

}  // namespace chrome
