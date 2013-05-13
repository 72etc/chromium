// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/extensions/api/enterprise_platform_keys_private/enterprise_platform_keys_private_api.h"

#include <string>

#include "base/prefs/pref_service.h"
#include "base/stringprintf.h"
#include "base/values.h"
#include "chrome/browser/chromeos/policy/stub_enterprise_install_attributes.h"
#include "chrome/browser/chromeos/settings/cros_settings_provider.h"
#include "chrome/browser/chromeos/settings/stub_cros_settings_provider.h"
#include "chrome/browser/extensions/extension_function_test_utils.h"
#include "chrome/browser/policy/cloud/cloud_policy_constants.h"
#include "chrome/common/pref_names.h"
#include "chrome/test/base/browser_with_test_window_test.h"
#include "chromeos/attestation/attestation_constants.h"
#include "chromeos/attestation/mock_attestation_flow.h"
#include "chromeos/cryptohome/async_method_caller.h"
#include "chromeos/cryptohome/mock_async_method_caller.h"
#include "chromeos/dbus/dbus_method_call_status.h"
#include "chromeos/dbus/mock_cryptohome_client.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/cros_system_api/dbus/service_constants.h"

using testing::_;
using testing::Invoke;
using testing::NiceMock;
using testing::Return;

namespace utils = extension_function_test_utils;

namespace extensions {
namespace {

void DoesKeyExistCallbackTrue(
    chromeos::attestation::AttestationKeyType key_type,
    const std::string& key_name,
    const chromeos::BoolDBusMethodCallback& callback) {
  callback.Run(chromeos::DBUS_METHOD_CALL_SUCCESS, true);
}

void DoesKeyExistCallbackFalse(
    chromeos::attestation::AttestationKeyType key_type,
    const std::string& key_name,
    const chromeos::BoolDBusMethodCallback& callback) {
  callback.Run(chromeos::DBUS_METHOD_CALL_SUCCESS, false);
}

void DoesKeyExistCallbackFailed(
    chromeos::attestation::AttestationKeyType key_type,
    const std::string& key_name,
    const chromeos::BoolDBusMethodCallback& callback) {
  callback.Run(chromeos::DBUS_METHOD_CALL_FAILURE, false);
}

void RegisterKeyCallbackTrue(
    chromeos::attestation::AttestationKeyType key_type,
    const std::string& key_name,
    const cryptohome::AsyncMethodCaller::Callback& callback) {
  callback.Run(true, cryptohome::MOUNT_ERROR_NONE);
}

void RegisterKeyCallbackFalse(
    chromeos::attestation::AttestationKeyType key_type,
    const std::string& key_name,
    const cryptohome::AsyncMethodCaller::Callback& callback) {
  callback.Run(false, cryptohome::MOUNT_ERROR_NONE);
}

void SignChallengeCallbackTrue(
    chromeos::attestation::AttestationKeyType key_type,
    const std::string& key_name,
    const std::string& domain,
    const std::string& device_id,
    chromeos::attestation::AttestationChallengeOptions options,
    const std::string& challenge,
    const cryptohome::AsyncMethodCaller::DataCallback& callback) {
  callback.Run(true, "response");
}

void SignChallengeCallbackFalse(
    chromeos::attestation::AttestationKeyType key_type,
    const std::string& key_name,
    const std::string& domain,
    const std::string& device_id,
    chromeos::attestation::AttestationChallengeOptions options,
    const std::string& challenge,
    const cryptohome::AsyncMethodCaller::DataCallback& callback) {
  callback.Run(false, "");
}

void GetCertificateCallbackTrue(
    chromeos::attestation::AttestationCertificateProfile certificate_profile,
    bool force_new_key,
    const chromeos::attestation::AttestationFlow::CertificateCallback&
        callback) {
  callback.Run(true, "certificate");
}

void GetCertificateCallbackFalse(
    chromeos::attestation::AttestationCertificateProfile certificate_profile,
    bool force_new_key,
    const chromeos::attestation::AttestationFlow::CertificateCallback&
        callback) {
  callback.Run(false, "");
}

class EPKPChallengeKeyTestBase : public BrowserWithTestWindowTest {
 protected:
  EPKPChallengeKeyTestBase()
      : extension_(utils::CreateEmptyExtension("")) {
    // Set up the default behavior of mocks.
    ON_CALL(mock_cryptohome_client_, TpmAttestationDoesKeyExist(_, _, _))
        .WillByDefault(Invoke(DoesKeyExistCallbackFalse));
    ON_CALL(mock_async_method_caller_, TpmAttestationRegisterKey(_, _, _))
        .WillByDefault(Invoke(RegisterKeyCallbackTrue));
    ON_CALL(mock_async_method_caller_,
            TpmAttestationSignEnterpriseChallenge(_, _, _, _, _, _, _))
        .WillByDefault(Invoke(SignChallengeCallbackTrue));
    ON_CALL(mock_attestation_flow_, GetCertificate(_, _, _))
        .WillByDefault(Invoke(GetCertificateCallbackTrue));

    // Set the Enterprise install attributes.
    stub_install_attributes_.SetDomain("google.com");
    stub_install_attributes_.SetRegistrationUser("test@google.com");
    stub_install_attributes_.SetDeviceId("device_id");
    stub_install_attributes_.SetMode(policy::DEVICE_MODE_ENTERPRISE);

    // Replace the default device setting provider with the stub.
    device_settings_provider_ = chromeos::CrosSettings::Get()->GetProvider(
        chromeos::kReportDeviceVersionInfo);
    EXPECT_TRUE(device_settings_provider_ != NULL);
    EXPECT_TRUE(chromeos::CrosSettings::Get()->
                RemoveSettingsProvider(device_settings_provider_));
    chromeos::CrosSettings::Get()->
        AddSettingsProvider(&stub_settings_provider_);

    // Set the device settings.
    stub_settings_provider_.Set(chromeos::kDeviceAttestationEnabled,
                                base::FundamentalValue(true));
  }

  virtual ~EPKPChallengeKeyTestBase() {
    EXPECT_TRUE(chromeos::CrosSettings::Get()->
                RemoveSettingsProvider(&stub_settings_provider_));
    chromeos::CrosSettings::Get()->
        AddSettingsProvider(device_settings_provider_);
  }

  NiceMock<chromeos::MockCryptohomeClient> mock_cryptohome_client_;
  NiceMock<cryptohome::MockAsyncMethodCaller> mock_async_method_caller_;
  NiceMock<chromeos::attestation::MockAttestationFlow> mock_attestation_flow_;
  scoped_refptr<extensions::Extension> extension_;
  policy::StubEnterpriseInstallAttributes stub_install_attributes_;
  chromeos::CrosSettingsProvider* device_settings_provider_;
  chromeos::StubCrosSettingsProvider stub_settings_provider_;
};

class EPKPChallengeMachineKeyTest : public EPKPChallengeKeyTestBase {
 protected:
  static const char kArgs[];

  EPKPChallengeMachineKeyTest()
      : func_(new EPKPChallengeMachineKey(&mock_cryptohome_client_,
                                          &mock_async_method_caller_,
                                          &mock_attestation_flow_,
                                          &stub_install_attributes_)) {
    func_->set_extension(extension_.get());
  }

  scoped_refptr<EPKPChallengeMachineKey> func_;
};

// Base 64 encoding of 'challenge'.
const char EPKPChallengeMachineKeyTest::kArgs[] = "[\"Y2hhbGxlbmdl\"]";

TEST_F(EPKPChallengeMachineKeyTest, ChallengeBadBase64) {
  EXPECT_EQ(EPKPChallengeKeyBase::kChallengeBadBase64Error,
            utils::RunFunctionAndReturnError(
                func_.get(), "[\"****\"]", browser()));
}

TEST_F(EPKPChallengeMachineKeyTest, NonEnterpriseDevice) {
  stub_install_attributes_.SetRegistrationUser("");

  EXPECT_EQ(EPKPChallengeMachineKey::kNonEnterpriseDeviceError,
            utils::RunFunctionAndReturnError(func_.get(), kArgs, browser()));
}

TEST_F(EPKPChallengeMachineKeyTest, DevicePolicyDisabled) {
  stub_settings_provider_.Set(chromeos::kDeviceAttestationEnabled,
                              base::FundamentalValue(false));

  EXPECT_EQ(EPKPChallengeKeyBase::kDevicePolicyDisabledError,
            utils::RunFunctionAndReturnError(func_.get(), kArgs, browser()));
}

TEST_F(EPKPChallengeMachineKeyTest, DoesKeyExistDbusFailed) {
  EXPECT_CALL(mock_cryptohome_client_, TpmAttestationDoesKeyExist(_, _, _))
      .WillRepeatedly(Invoke(DoesKeyExistCallbackFailed));

  EXPECT_EQ(base::StringPrintf(
      EPKPChallengeMachineKey::kGetCertificateFailedError, 1),
            utils::RunFunctionAndReturnError(func_.get(), kArgs, browser()));
}

TEST_F(EPKPChallengeMachineKeyTest, GetCertificateFailed) {
  EXPECT_CALL(mock_attestation_flow_, GetCertificate(_, _, _))
      .WillRepeatedly(Invoke(GetCertificateCallbackFalse));

  EXPECT_EQ(base::StringPrintf(
      EPKPChallengeMachineKey::kGetCertificateFailedError, 3),
            utils::RunFunctionAndReturnError(func_.get(), kArgs, browser()));
}

TEST_F(EPKPChallengeMachineKeyTest, SignChallengeFailed) {
  EXPECT_CALL(mock_async_method_caller_,
              TpmAttestationSignEnterpriseChallenge(_, _, _, _, _, _, _))
      .WillRepeatedly(Invoke(SignChallengeCallbackFalse));

  EXPECT_EQ(EPKPChallengeKeyBase::kSignChallengeFailedError,
            utils::RunFunctionAndReturnError(func_.get(), kArgs, browser()));
}

TEST_F(EPKPChallengeMachineKeyTest, KeyExists) {
  EXPECT_CALL(mock_cryptohome_client_, TpmAttestationDoesKeyExist(_, _, _))
      .WillRepeatedly(Invoke(DoesKeyExistCallbackTrue));
  // GetCertificate must not be called if the key exists.
  EXPECT_CALL(mock_attestation_flow_, GetCertificate(_, _, _))
      .Times(0);

  EXPECT_TRUE(utils::RunFunction(func_.get(), kArgs, browser(), utils::NONE));
}

TEST_F(EPKPChallengeMachineKeyTest, Success) {
  // GetCertificate must be called exactly once.
  EXPECT_CALL(mock_attestation_flow_,
              GetCertificate(
                  chromeos::attestation::PROFILE_ENTERPRISE_MACHINE_CERTIFICATE,
                  _, _))
      .Times(1);
  // SignEnterpriseChallenge must be called exactly once.
  EXPECT_CALL(mock_async_method_caller_,
              TpmAttestationSignEnterpriseChallenge(
                  chromeos::attestation::KEY_DEVICE, "attest-ent-machine",
                  "google.com", "device_id", _, "challenge", _))
      .Times(1);

  scoped_ptr<base::Value> value = utils::RunFunctionAndReturnSingleResult(
      func_.get(), kArgs, browser(), utils::NONE);

  std::string response;
  value->GetAsString(&response);
  EXPECT_EQ("cmVzcG9uc2U=" /* Base64 encoding of 'response' */, response);
}

class EPKPChallengeUserKeyTest : public EPKPChallengeKeyTestBase {
 protected:
  static const char kArgs[];

  EPKPChallengeUserKeyTest() :
      func_(new EPKPChallengeUserKey(&mock_cryptohome_client_,
                                     &mock_async_method_caller_,
                                     &mock_attestation_flow_,
                                     &stub_install_attributes_)) {
    func_->set_extension(extension_.get());
  }

  virtual void SetUp() OVERRIDE {
    EPKPChallengeKeyTestBase::SetUp();

    // Set the user preferences.
    prefs_ = browser()->profile()->GetPrefs();
    prefs_->SetBoolean(prefs::kAttestationEnabled, true);
    base::ListValue whitelist;
    whitelist.AppendString(extension_->id());
    prefs_->Set(prefs::kAttestationExtensionWhitelist, whitelist);
    prefs_->SetString(prefs::kGoogleServicesUsername, "test@google.com");
  }

  scoped_refptr<EPKPChallengeUserKey> func_;
  PrefService* prefs_;
};

// Base 64 encoding of 'challenge'
const char EPKPChallengeUserKeyTest::kArgs[] = "[\"Y2hhbGxlbmdl\", true]";

TEST_F(EPKPChallengeUserKeyTest, ChallengeBadBase64) {
  EXPECT_EQ(EPKPChallengeKeyBase::kChallengeBadBase64Error,
            utils::RunFunctionAndReturnError(
                func_.get(), "[\"****\", true]", browser()));
}

TEST_F(EPKPChallengeUserKeyTest, UserPolicyDisabled) {
  prefs_->SetBoolean(prefs::kAttestationEnabled, false);

  EXPECT_EQ(EPKPChallengeUserKey::kUserPolicyDisabledError,
            utils::RunFunctionAndReturnError(func_.get(), kArgs, browser()));
}

TEST_F(EPKPChallengeUserKeyTest, ExtensionNotWhitelisted) {
  base::ListValue empty_whitelist;
  prefs_->Set(prefs::kAttestationExtensionWhitelist, empty_whitelist);

  EXPECT_EQ(EPKPChallengeUserKey::kExtensionNotWhitelistedError,
            utils::RunFunctionAndReturnError(func_.get(), kArgs, browser()));
}

TEST_F(EPKPChallengeUserKeyTest, DomainsDontMatch) {
  prefs_->SetString(prefs::kGoogleServicesUsername, "test@chromium.org");

  EXPECT_EQ(base::StringPrintf(EPKPChallengeUserKey::kDomainsDontMatchError,
                               "chromium.org", "google.com"),
            utils::RunFunctionAndReturnError(func_.get(), kArgs, browser()));
}

TEST_F(EPKPChallengeUserKeyTest, DevicePolicyDisabled) {
  stub_settings_provider_.Set(chromeos::kDeviceAttestationEnabled,
                              base::FundamentalValue(false));

  EXPECT_EQ(EPKPChallengeKeyBase::kDevicePolicyDisabledError,
            utils::RunFunctionAndReturnError(func_.get(), kArgs, browser()));
}

TEST_F(EPKPChallengeUserKeyTest, DoesKeyExistDbusFailed) {
  EXPECT_CALL(mock_cryptohome_client_, TpmAttestationDoesKeyExist(_, _, _))
      .WillRepeatedly(Invoke(DoesKeyExistCallbackFailed));

  EXPECT_EQ(base::StringPrintf(
      EPKPChallengeUserKey::kGetCertificateFailedError, 1),
            utils::RunFunctionAndReturnError(func_.get(), kArgs, browser()));
}

TEST_F(EPKPChallengeUserKeyTest, GetCertificateFailed) {
  EXPECT_CALL(mock_attestation_flow_, GetCertificate(_, _, _))
      .WillRepeatedly(Invoke(GetCertificateCallbackFalse));

  EXPECT_EQ(base::StringPrintf(
      EPKPChallengeUserKey::kGetCertificateFailedError, 3),
            utils::RunFunctionAndReturnError(func_.get(), kArgs, browser()));
}

TEST_F(EPKPChallengeUserKeyTest, SignChallengeFailed) {
  EXPECT_CALL(mock_async_method_caller_,
              TpmAttestationSignEnterpriseChallenge(_, _, _, _, _, _, _))
      .WillRepeatedly(Invoke(SignChallengeCallbackFalse));

  EXPECT_EQ(EPKPChallengeKeyBase::kSignChallengeFailedError,
            utils::RunFunctionAndReturnError(func_.get(), kArgs, browser()));
}

TEST_F(EPKPChallengeUserKeyTest, KeyRegistrationFailed) {
  EXPECT_CALL(mock_async_method_caller_, TpmAttestationRegisterKey(_, _, _))
      .WillRepeatedly(Invoke(RegisterKeyCallbackFalse));

  EXPECT_EQ(EPKPChallengeUserKey::kKeyRegistrationFailedError,
            utils::RunFunctionAndReturnError(func_.get(), kArgs, browser()));
}

TEST_F(EPKPChallengeUserKeyTest, KeyExists) {
  EXPECT_CALL(mock_cryptohome_client_, TpmAttestationDoesKeyExist(_, _, _))
      .WillRepeatedly(Invoke(DoesKeyExistCallbackTrue));
  // GetCertificate must not be called if the key exists.
  EXPECT_CALL(mock_attestation_flow_, GetCertificate(_, _, _))
      .Times(0);

  EXPECT_TRUE(utils::RunFunction(func_.get(), kArgs, browser(), utils::NONE));
}

TEST_F(EPKPChallengeUserKeyTest, KeyNotRegistered) {
  EXPECT_CALL(mock_async_method_caller_, TpmAttestationRegisterKey(_, _, _))
      .Times(0);

  EXPECT_TRUE(utils::RunFunction(
      func_.get(), "[\"Y2hhbGxlbmdl\", false]", browser(), utils::NONE));
}

TEST_F(EPKPChallengeUserKeyTest, PersonalDevice) {
  stub_install_attributes_.SetRegistrationUser("");

  // Currently personal devices are not supported.
  EXPECT_EQ("Failed to get Enterprise user certificate. Error code = 2",
            utils::RunFunctionAndReturnError(func_.get(), kArgs, browser()));
}

TEST_F(EPKPChallengeUserKeyTest, Success) {
  // GetCertificate must be called exactly once.
  EXPECT_CALL(mock_attestation_flow_,
              GetCertificate(
                  chromeos::attestation::PROFILE_ENTERPRISE_USER_CERTIFICATE,
                  _, _))
      .Times(1);
  // SignEnterpriseChallenge must be called exactly once.
  EXPECT_CALL(mock_async_method_caller_,
              TpmAttestationSignEnterpriseChallenge(
                  chromeos::attestation::KEY_USER, "attest-ent-user",
                  "google.com", "device_id", _, "challenge", _))
      .Times(1);
  // RegisterKey must be called exactly once.
  EXPECT_CALL(mock_async_method_caller_,
              TpmAttestationRegisterKey(chromeos::attestation::KEY_USER,
                                        "attest-ent-user", _))
      .Times(1);

  scoped_ptr<base::Value> value = utils::RunFunctionAndReturnSingleResult(
      func_.get(), kArgs, browser(), utils::NONE);

  std::string response;
  value->GetAsString(&response);
  EXPECT_EQ("cmVzcG9uc2U=" /* Base64 encoding of 'response' */, response);
}

}  // namespace
}  // namespace extensions
