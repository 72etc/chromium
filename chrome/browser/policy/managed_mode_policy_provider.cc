// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/policy/managed_mode_policy_provider.h"

#include "base/json/json_reader.h"
#include "base/json/json_writer.h"
#include "base/prefs/json_pref_store.h"
#include "base/threading/sequenced_worker_pool.h"
#include "chrome/browser/managed_mode/managed_mode_url_filter.h"
#include "chrome/browser/policy/policy_bundle.h"
#include "chrome/browser/prefs/incognito_mode_prefs.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/common/chrome_constants.h"
#include "content/public/browser/browser_thread.h"
#include "policy/policy_constants.h"
#include "sync/api/sync_change.h"
#include "sync/protocol/sync.pb.h"

using base::DictionaryValue;
using base::Value;
using content::BrowserThread;
using syncer::MANAGED_USER_SETTINGS;
using syncer::ModelType;
using syncer::SyncChange;
using syncer::SyncChangeList;
using syncer::SyncChangeProcessor;
using syncer::SyncData;
using syncer::SyncDataList;
using syncer::SyncError;
using syncer::SyncErrorFactory;
using syncer::SyncMergeResult;

namespace {

SyncData CreateSyncDataForLocalPolicy(const std::string& name,
                                      const Value* value) {
  std::string json_value;
  base::JSONWriter::Write(value, &json_value);
  ::sync_pb::EntitySpecifics specifics;
  specifics.mutable_managed_user_setting()->set_name(name);
  specifics.mutable_managed_user_setting()->set_value(json_value);
  return SyncData::CreateLocalData(name, name, specifics);
}

}  // namespace

namespace policy {

// static
const char ManagedModePolicyProvider::kPolicies[] = "policies";

// static
scoped_ptr<ManagedModePolicyProvider> ManagedModePolicyProvider::Create(
    Profile* profile,
    base::SequencedTaskRunner* sequenced_task_runner,
    bool force_load) {
  base::FilePath path =
      profile->GetPath().Append(chrome::kManagedModePolicyFilename);
  JsonPrefStore* pref_store = new JsonPrefStore(path, sequenced_task_runner);
  // Load the data synchronously if needed (when creating profiles on startup).
  if (force_load)
    pref_store->ReadPrefs();
  else
    pref_store->ReadPrefsAsync(NULL);

  return make_scoped_ptr(new ManagedModePolicyProvider(pref_store));
}

ManagedModePolicyProvider::ManagedModePolicyProvider(
    PersistentPrefStore* pref_store)
    : store_(pref_store) {
  store_->AddObserver(this);
  if (store_->IsInitializationComplete())
    UpdatePolicyFromCache();
}

ManagedModePolicyProvider::~ManagedModePolicyProvider() {}

void ManagedModePolicyProvider::InitDefaults() {
  DCHECK(store_->IsInitializationComplete());
  if (GetCachedPolicy())
    return;

  DictionaryValue* dict = new DictionaryValue;
  dict->SetInteger(policy::key::kContentPackDefaultFilteringBehavior,
                   ManagedModeURLFilter::ALLOW);
  dict->SetBoolean(policy::key::kForceSafeSearch, true);
  dict->SetBoolean(policy::key::kSigninAllowed, false);
  dict->SetBoolean(policy::key::kAllowDeletingBrowserHistory, false);
  dict->SetInteger(policy::key::kIncognitoModeAvailability,
                   IncognitoModePrefs::DISABLED);

  store_->SetValue(kPolicies, dict);
  UpdatePolicyFromCache();
}

void ManagedModePolicyProvider::InitDefaultsForTesting(
    scoped_ptr<DictionaryValue> dict) {
  DCHECK(store_->IsInitializationComplete());
  DCHECK(!GetCachedPolicy());
  store_->SetValue(kPolicies, dict.release());
  UpdatePolicyFromCache();
}

const DictionaryValue* ManagedModePolicyProvider::GetPolicies() const {
  DictionaryValue* dict = GetCachedPolicy();
  DCHECK(dict);
  return dict;
}

const Value* ManagedModePolicyProvider::GetPolicy(
    const std::string& key) const {
  DictionaryValue* dict = GetCachedPolicy();
  Value* value = NULL;

  // Returns NULL if the key doesn't exist.
  dict->GetWithoutPathExpansion(key, &value);
  return value;
}

scoped_ptr<DictionaryValue> ManagedModePolicyProvider::GetPolicyDictionary(
    const std::string& key) {
  const Value* value = GetPolicy(key);
  const DictionaryValue* dict = NULL;
  if (value && value->GetAsDictionary(&dict))
    return make_scoped_ptr(dict->DeepCopy());

  return make_scoped_ptr(new DictionaryValue);
}

void ManagedModePolicyProvider::SetPolicy(const std::string& key,
                                          scoped_ptr<Value> value) {
  DictionaryValue* dict = GetCachedPolicy();
  if (value)
    dict->SetWithoutPathExpansion(key, value.release());
  else
    dict->RemoveWithoutPathExpansion(key, NULL);

  // TODO(bauerb): Report changes to sync.
  store_->ReportValueChanged(kPolicies);
  UpdatePolicyFromCache();
}

void ManagedModePolicyProvider::Shutdown() {
  store_->RemoveObserver(this);
  ConfigurationPolicyProvider::Shutdown();
}

void ManagedModePolicyProvider::RefreshPolicies() {
  UpdatePolicyFromCache();
}

bool ManagedModePolicyProvider::IsInitializationComplete(
    PolicyDomain domain) const {
  if (domain == POLICY_DOMAIN_CHROME)
    return store_->IsInitializationComplete();
  return true;
}

void ManagedModePolicyProvider::OnPrefValueChanged(const std::string& key) {}

void ManagedModePolicyProvider::OnInitializationCompleted(bool success) {
  DCHECK(success);
  UpdatePolicyFromCache();
}

SyncMergeResult ManagedModePolicyProvider::MergeDataAndStartSyncing(
    ModelType type,
    const SyncDataList& initial_sync_data,
    scoped_ptr<SyncChangeProcessor> sync_processor,
    scoped_ptr<SyncErrorFactory> error_handler) {
  DCHECK_EQ(MANAGED_USER_SETTINGS, type);
  sync_processor_ = sync_processor.Pass();
  error_handler_ = error_handler.Pass();
  DictionaryValue* policy = GetCachedPolicy();
  base::JSONReader reader;
  std::set<std::string> seen_keys;
  for (SyncDataList::const_iterator it = initial_sync_data.begin();
       it != initial_sync_data.end(); ++it) {
    DCHECK_EQ(MANAGED_USER_SETTINGS, it->GetDataType());
    const ::sync_pb::ManagedUserSettingSpecifics& managed_user_setting =
        it->GetSpecifics().managed_user_setting();
    Value* value = reader.Read(managed_user_setting.value());
    seen_keys.insert(managed_user_setting.name());
    policy->SetWithoutPathExpansion(managed_user_setting.name(), value);
  }

  SyncChangeList change_list;
  for (DictionaryValue::Iterator it(*policy); !it.IsAtEnd(); it.Advance()) {
    // Send all local policies that are not in the initial sync list
    // to the server.
    if (seen_keys.find(it.key()) != seen_keys.end())
      continue;

    SyncData data = CreateSyncDataForLocalPolicy(it.key(), &it.value());
    change_list.push_back(SyncChange(FROM_HERE, SyncChange::ACTION_ADD, data));
  }
  sync_processor_->ProcessSyncChanges(FROM_HERE, change_list);

  store_->ReportValueChanged(kPolicies);
  UpdatePolicyFromCache();

  SyncMergeResult result(MANAGED_USER_SETTINGS);
  return result;
}

void ManagedModePolicyProvider::StopSyncing(ModelType type) {
  DCHECK_EQ(syncer::MANAGED_USER_SETTINGS, type);
  sync_processor_.reset();
  error_handler_.reset();
}

SyncDataList ManagedModePolicyProvider::GetAllSyncData(ModelType type) const {
  DCHECK_EQ(syncer::MANAGED_USER_SETTINGS, type);
  SyncDataList data;
  DictionaryValue* policy = GetCachedPolicy();
  for (DictionaryValue::Iterator it(*policy); !it.IsAtEnd(); it.Advance()) {
    data.push_back(CreateSyncDataForLocalPolicy(it.key(), &it.value()));
  }
  return data;
}

SyncError ManagedModePolicyProvider::ProcessSyncChanges(
    const tracked_objects::Location& from_here,
    const SyncChangeList& change_list) {
  SyncError error;
  DictionaryValue* policy = GetCachedPolicy();
  base::JSONReader reader;
  for (SyncChangeList::const_iterator it = change_list.begin();
       it != change_list.end(); ++it) {
    SyncData data = it->sync_data();
    DCHECK_EQ(MANAGED_USER_SETTINGS, data.GetDataType());
    const ::sync_pb::ManagedUserSettingSpecifics& managed_user_setting =
        data.GetSpecifics().managed_user_setting();
    switch (it->change_type()) {
      case SyncChange::ACTION_ADD:
      case SyncChange::ACTION_UPDATE: {
        Value* value = reader.Read(managed_user_setting.value());
        if (policy->HasKey(managed_user_setting.name())) {
          DLOG_IF(WARNING, it->change_type() == SyncChange::ACTION_ADD)
              << "Value for key " << managed_user_setting.name()
              << " already exists";
        } else {
          DLOG_IF(WARNING, it->change_type() == SyncChange::ACTION_UPDATE)
              << "Value for key " << managed_user_setting.name()
              << " doesn't exist yet";
        }
        policy->SetWithoutPathExpansion(managed_user_setting.name(), value);
        break;
      }
      case SyncChange::ACTION_DELETE: {
        DLOG_IF(WARNING, !policy->HasKey(managed_user_setting.name()))
            << "Trying to delete non-existing key "
            << managed_user_setting.name();
        policy->RemoveWithoutPathExpansion(managed_user_setting.name(), NULL);
        break;
      }
      case SyncChange::ACTION_INVALID: {
        NOTREACHED();
        break;
      }
    }
  }
  return error;
}

DictionaryValue* ManagedModePolicyProvider::GetCachedPolicy() const {
  Value* value = NULL;
  if (!store_->GetMutableValue(kPolicies, &value))
    return NULL;

  DictionaryValue* dict = NULL;
  bool success = value->GetAsDictionary(&dict);
  DCHECK(success);
  return dict;
}

void ManagedModePolicyProvider::UpdatePolicyFromCache() {
  scoped_ptr<PolicyBundle> policy_bundle(new PolicyBundle);
  DictionaryValue* policies = GetCachedPolicy();
  if (policies) {
    PolicyMap* policy_map = &policy_bundle->Get(
        PolicyNamespace(POLICY_DOMAIN_CHROME, std::string()));
    policy_map->LoadFrom(policies, POLICY_LEVEL_MANDATORY, POLICY_SCOPE_USER);
  }
  UpdatePolicy(policy_bundle.Pass());
}

}  // namespace policy
