// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/browser_context_keyed_service/browser_context_keyed_base_factory.h"

#include "base/prefs/pref_service.h"
#include "components/browser_context_keyed_service/browser_context_dependency_manager.h"
#include "components/user_prefs/pref_registry_syncable.h"
#include "components/user_prefs/user_prefs.h"
#include "content/public/browser/browser_context.h"

ProfileKeyedBaseFactory::ProfileKeyedBaseFactory(
    const char* name, ProfileDependencyManager* manager)
    : dependency_manager_(manager)
#ifndef NDEBUG
    , service_name_(name)
#endif
{
  dependency_manager_->AddComponent(this);
}

ProfileKeyedBaseFactory::~ProfileKeyedBaseFactory() {
  dependency_manager_->RemoveComponent(this);
}

void ProfileKeyedBaseFactory::DependsOn(ProfileKeyedBaseFactory* rhs) {
  dependency_manager_->AddEdge(rhs, this);
}

content::BrowserContext* ProfileKeyedBaseFactory::GetBrowserContextToUse(
    content::BrowserContext* context) const {
  DCHECK(CalledOnValidThread());

#ifndef NDEBUG
  dependency_manager_->AssertProfileWasntDestroyed(context);
#endif

  // Safe default for the Incognito mode: no service.
  if (context->IsOffTheRecord())
    return NULL;

  return context;
}

void ProfileKeyedBaseFactory::RegisterUserPrefsOnProfile(
    content::BrowserContext* profile) {
  // Safe timing for pref registration is hard. Previously, we made Profile
  // responsible for all pref registration on every service that used
  // Profile. Now we don't and there are timing issues.
  //
  // With normal profiles, prefs can simply be registered at
  // ProfileDependencyManager::CreateProfileServices time. With incognito
  // profiles, we just never register since incognito profiles share the same
  // pref services with their parent profiles.
  //
  // TestingProfiles throw a wrench into the mix, in that some tests will
  // swap out the PrefService after we've registered user prefs on the original
  // PrefService. Test code that does this is responsible for either manually
  // invoking RegisterUserPrefs() on the appropriate ProfileKeyedServiceFactory
  // associated with the prefs they need, or they can use SetTestingFactory()
  // and create a service (since service creation with a factory method causes
  // registration to happen at service creation time).
  //
  // Now that services are responsible for declaring their preferences, we have
  // to enforce a uniquenes check here because some tests create one profile and
  // multiple services of the same type attached to that profile (serially, not
  // parallel) and we don't want to register multiple times on the same profile.
  DCHECK(!profile->IsOffTheRecord());

  std::set<content::BrowserContext*>::iterator it =
      registered_preferences_.find(profile);
  if (it == registered_preferences_.end()) {
    PrefService* prefs = components::UserPrefs::Get(profile);
    user_prefs::PrefRegistrySyncable* registry =
        static_cast<user_prefs::PrefRegistrySyncable*>(
            prefs->DeprecatedGetPrefRegistry());
    RegisterUserPrefs(registry);
    registered_preferences_.insert(profile);
  }
}

bool ProfileKeyedBaseFactory::ServiceIsCreatedWithProfile() const {
  return false;
}

bool ProfileKeyedBaseFactory::ServiceIsNULLWhileTesting() const {
  return false;
}

void ProfileKeyedBaseFactory::ProfileDestroyed(
    content::BrowserContext* profile) {
  // While object destruction can be customized in ways where the object is
  // only dereferenced, this still must run on the UI thread.
  DCHECK(CalledOnValidThread());

  registered_preferences_.erase(profile);
}

bool ProfileKeyedBaseFactory::ArePreferencesSetOn(
    content::BrowserContext* profile) const {
  return registered_preferences_.find(profile) !=
      registered_preferences_.end();
}

void ProfileKeyedBaseFactory::MarkPreferencesSetOn(
    content::BrowserContext* profile) {
  DCHECK(!ArePreferencesSetOn(profile));
  registered_preferences_.insert(profile);
}
