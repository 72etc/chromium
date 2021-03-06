// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chromeos/dbus/fake_session_manager_client.h"

#include "base/bind.h"
#include "base/location.h"
#include "base/message_loop.h"
#include "base/string_util.h"

namespace chromeos {

FakeSessionManagerClient::FakeSessionManagerClient()
  : emit_login_prompt_ready_call_count_(0) ,
    notify_lock_screen_shown_call_count_(0),
    notify_lock_screen_dismissed_call_count_(0){
}

FakeSessionManagerClient::~FakeSessionManagerClient() {
}

void FakeSessionManagerClient::AddObserver(Observer* observer) {
  observers_.AddObserver(observer);
}

void FakeSessionManagerClient::RemoveObserver(Observer* observer) {
  observers_.RemoveObserver(observer);
}

bool FakeSessionManagerClient::HasObserver(Observer* observer) {
  return observers_.HasObserver(observer);
}

void FakeSessionManagerClient::EmitLoginPromptReady() {
  emit_login_prompt_ready_call_count_++;
}

void FakeSessionManagerClient::EmitLoginPromptVisible() {
}

void FakeSessionManagerClient::RestartJob(int pid,
                                          const std::string& command_line) {
}

void FakeSessionManagerClient::RestartEntd() {
}

void FakeSessionManagerClient::StartSession(const std::string& user_email) {
}

void FakeSessionManagerClient::StopSession() {
}

void FakeSessionManagerClient::StartDeviceWipe() {
}

void FakeSessionManagerClient::RequestLockScreen() {
}

void FakeSessionManagerClient::NotifyLockScreenShown() {
  notify_lock_screen_shown_call_count_++;
}

void FakeSessionManagerClient::RequestUnlockScreen() {
}

void FakeSessionManagerClient::NotifyLockScreenDismissed() {
  notify_lock_screen_dismissed_call_count_++;
}

void FakeSessionManagerClient::RetrieveDevicePolicy(
    const RetrievePolicyCallback& callback) {
  MessageLoop::current()->PostTask(FROM_HERE,
                                   base::Bind(callback, device_policy_));
}

void FakeSessionManagerClient::RetrievePolicyForUser(
    const std::string& username,
    const RetrievePolicyCallback& callback) {
  MessageLoop::current()->PostTask(
      FROM_HERE, base::Bind(callback, user_policies_[username]));
}

void FakeSessionManagerClient::RetrieveDeviceLocalAccountPolicy(
    const std::string& account_id,
    const RetrievePolicyCallback& callback) {
  MessageLoop::current()->PostTask(
      FROM_HERE,
      base::Bind(callback, device_local_account_policy_[account_id]));
}

void FakeSessionManagerClient::StoreDevicePolicy(
    const std::string& policy_blob,
    const StorePolicyCallback& callback) {
  device_policy_ = policy_blob;
  MessageLoop::current()->PostTask(FROM_HERE, base::Bind(callback, true));
  FOR_EACH_OBSERVER(Observer, observers_, PropertyChangeComplete(true));
}

void FakeSessionManagerClient::StorePolicyForUser(
    const std::string& username,
    const std::string& policy_blob,
    const std::string& policy_key,
    const StorePolicyCallback& callback) {
  user_policies_[username] = policy_blob;
  MessageLoop::current()->PostTask(FROM_HERE, base::Bind(callback, true));
}

void FakeSessionManagerClient::StoreDeviceLocalAccountPolicy(
    const std::string& account_id,
    const std::string& policy_blob,
    const StorePolicyCallback& callback) {
  device_local_account_policy_[account_id] = policy_blob;
  MessageLoop::current()->PostTask(FROM_HERE, base::Bind(callback, true));
}

const std::string& FakeSessionManagerClient::device_policy() const {
  return device_policy_;
}

void FakeSessionManagerClient::set_device_policy(
    const std::string& policy_blob) {
  device_policy_ = policy_blob;
}

const std::string& FakeSessionManagerClient::user_policy(
    const std::string& username) const {
  std::map<std::string, std::string>::const_iterator it =
      user_policies_.find(username);
  return it == user_policies_.end() ? EmptyString() : it->second;
}

void FakeSessionManagerClient::set_user_policy(const std::string& username,
                                               const std::string& policy_blob) {
  user_policies_[username] = policy_blob;
}

const std::string& FakeSessionManagerClient::device_local_account_policy(
    const std::string& account_id) const {
  std::map<std::string, std::string>::const_iterator entry =
      device_local_account_policy_.find(account_id);
  return entry != device_local_account_policy_.end() ? entry->second
                                                     : EmptyString();
}

void FakeSessionManagerClient::set_device_local_account_policy(
    const std::string& account_id,
    const std::string& policy_blob) {
  device_local_account_policy_[account_id] = policy_blob;
}

void FakeSessionManagerClient::OnPropertyChangeComplete(bool success) {
  FOR_EACH_OBSERVER(Observer, observers_, PropertyChangeComplete(success));
}

}  // namespace chromeos
