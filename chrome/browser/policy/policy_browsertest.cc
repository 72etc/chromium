// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>

#include "base/file_path.h"
#include "chrome/app/chrome_command_ids.h"
#include "chrome/browser/browser_process.h"
#include "chrome/browser/infobars/infobar_tab_helper.h"
#include "chrome/browser/policy/browser_policy_connector.h"
#include "chrome/browser/policy/mock_configuration_policy_provider.h"
#include "chrome/browser/policy/policy_map.h"
#include "chrome/browser/prefs/pref_service.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/translate/translate_infobar_delegate.h"
#include "chrome/browser/ui/bookmarks/bookmark_bar.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/browser_commands.h"
#include "chrome/browser/ui/browser_list.h"
#include "chrome/browser/ui/browser_tabstrip.h"
#include "chrome/browser/ui/tab_contents/tab_contents.h"
#include "chrome/common/chrome_notification_types.h"
#include "chrome/common/pref_names.h"
#include "chrome/test/base/in_process_browser_test.h"
#include "chrome/test/base/ui_test_utils.h"
#include "content/public/browser/notification_service.h"
#include "content/public/test/browser_test_utils.h"
#include "googleurl/src/gurl.h"
#include "policy/policy_constants.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

using testing::Return;

namespace policy {

namespace {

const char kURL[] = "http://example.com";
const char kCookieValue[] = "converted=true";
// Assigned to Philip J. Fry to fix eventually.
const char kCookieOptions[] = ";expires=Wed Jan 01 3000 00:00:00 GMT";

}  // namespace

class PolicyTest : public InProcessBrowserTest {
 protected:
  PolicyTest() {}

  virtual void SetUpInProcessBrowserTestFixture() OVERRIDE {
    EXPECT_CALL(provider_, IsInitializationComplete())
        .WillRepeatedly(Return(true));
    BrowserPolicyConnector::SetPolicyProviderForTesting(&provider_);
  }

  MockConfigurationPolicyProvider provider_;
};

IN_PROC_BROWSER_TEST_F(PolicyTest, BookmarkBarEnabled) {
  // Test starts in about:blank.
  PrefService* prefs = browser()->profile()->GetPrefs();
  EXPECT_FALSE(prefs->IsManagedPreference(prefs::kShowBookmarkBar));
  EXPECT_FALSE(prefs->GetBoolean(prefs::kShowBookmarkBar));
  EXPECT_EQ(BookmarkBar::HIDDEN, browser()->bookmark_bar_state());

  PolicyMap policies;
  policies.Set(key::kBookmarkBarEnabled, POLICY_LEVEL_MANDATORY,
               POLICY_SCOPE_USER, base::Value::CreateBooleanValue(true));
  provider_.UpdateChromePolicy(policies);
  EXPECT_TRUE(prefs->IsManagedPreference(prefs::kShowBookmarkBar));
  EXPECT_TRUE(prefs->GetBoolean(prefs::kShowBookmarkBar));
  EXPECT_EQ(BookmarkBar::SHOW, browser()->bookmark_bar_state());

  // The NTP has special handling of the bookmark bar.
  ui_test_utils::NavigateToURL(browser(), GURL("chrome://newtab"));
  EXPECT_EQ(BookmarkBar::SHOW, browser()->bookmark_bar_state());

  policies.Set(key::kBookmarkBarEnabled, POLICY_LEVEL_MANDATORY,
               POLICY_SCOPE_USER, base::Value::CreateBooleanValue(false));
  provider_.UpdateChromePolicy(policies);
  EXPECT_TRUE(prefs->IsManagedPreference(prefs::kShowBookmarkBar));
  EXPECT_FALSE(prefs->GetBoolean(prefs::kShowBookmarkBar));
  // The bookmark bar is hidden in the NTP when disabled by policy.
  EXPECT_EQ(BookmarkBar::HIDDEN, browser()->bookmark_bar_state());

  policies.Clear();
  provider_.UpdateChromePolicy(policies);
  EXPECT_FALSE(prefs->IsManagedPreference(prefs::kShowBookmarkBar));
  EXPECT_FALSE(prefs->GetBoolean(prefs::kShowBookmarkBar));
  // The bookmark bar is shown detached in the NTP, when disabled by prefs only.
  EXPECT_EQ(BookmarkBar::DETACHED, browser()->bookmark_bar_state());
}

IN_PROC_BROWSER_TEST_F(PolicyTest, PRE_PRE_ClearSiteDataOnExit) {
  Profile* profile = browser()->profile();
  GURL url(kURL);
  // No cookies at startup.
  EXPECT_TRUE(content::GetCookies(profile, url).empty());
  // Set a cookie now.
  std::string value = std::string(kCookieValue) + std::string(kCookieOptions);
  EXPECT_TRUE(content::SetCookie(profile, url, value));
  // Verify it was set.
  EXPECT_EQ(kCookieValue, GetCookies(profile, url));
}

IN_PROC_BROWSER_TEST_F(PolicyTest, PRE_ClearSiteDataOnExit) {
  // Verify that the cookie persists across restarts.
  EXPECT_EQ(kCookieValue, GetCookies(browser()->profile(), GURL(kURL)));
  // Now set the policy and the cookie should be gone after another restart.
  PolicyMap policies;
  policies.Set(key::kClearSiteDataOnExit, POLICY_LEVEL_MANDATORY,
               POLICY_SCOPE_USER, base::Value::CreateBooleanValue(true));
  provider_.UpdateChromePolicy(policies);
}

IN_PROC_BROWSER_TEST_F(PolicyTest, ClearSiteDataOnExit) {
  // Verify that the cookie is gone.
  EXPECT_TRUE(GetCookies(browser()->profile(), GURL(kURL)).empty());
}

IN_PROC_BROWSER_TEST_F(PolicyTest, IncognitoEnabled) {
  // Disable incognito via policy and verify that incognito windows can't be
  // opened.
  EXPECT_EQ(1u, BrowserList::size());
  EXPECT_FALSE(BrowserList::IsOffTheRecordSessionActive());
  PolicyMap policies;
  policies.Set(key::kIncognitoEnabled, POLICY_LEVEL_MANDATORY,
               POLICY_SCOPE_USER, base::Value::CreateBooleanValue(false));
  provider_.UpdateChromePolicy(policies);
  EXPECT_FALSE(chrome::ExecuteCommand(browser(), IDC_NEW_INCOGNITO_WINDOW));
  EXPECT_EQ(1u, BrowserList::size());
  EXPECT_FALSE(BrowserList::IsOffTheRecordSessionActive());

  // Enable via policy and verify that incognito windows can be opened.
  policies.Set(key::kIncognitoEnabled, POLICY_LEVEL_MANDATORY,
               POLICY_SCOPE_USER, base::Value::CreateBooleanValue(true));
  provider_.UpdateChromePolicy(policies);
  EXPECT_TRUE(chrome::ExecuteCommand(browser(), IDC_NEW_INCOGNITO_WINDOW));
  EXPECT_EQ(2u, BrowserList::size());
  EXPECT_TRUE(BrowserList::IsOffTheRecordSessionActive());
}

IN_PROC_BROWSER_TEST_F(PolicyTest, TranslateEnabled) {
  // Get the |infobar_helper|, and verify that there are no infobars on startup.
  content::WebContents* contents = chrome::GetActiveWebContents(browser());
  ASSERT_TRUE(contents);
  TabContents* tab_contents = TabContents::FromWebContents(contents);
  ASSERT_TRUE(tab_contents);
  InfoBarTabHelper* infobar_helper = tab_contents->infobar_tab_helper();
  ASSERT_TRUE(infobar_helper);
  EXPECT_EQ(0u, infobar_helper->GetInfoBarCount());

  // Force enable the translate feature.
  PolicyMap policies;
  policies.Set(key::kTranslateEnabled, POLICY_LEVEL_MANDATORY,
               POLICY_SCOPE_USER, base::Value::CreateBooleanValue(true));
  provider_.UpdateChromePolicy(policies);
  // Instead of waiting for NOTIFICATION_TAB_CONTENTS_INFOBAR_ADDED, this test
  // waits for NOTIFICATION_TAB_LANGUAGE_DETERMINED because that's what the
  // TranslateManager observes. This allows checking that an infobar is NOT
  // shown below, without polling for infobars for some indeterminate amount
  // of time.
  GURL url = ui_test_utils::GetTestUrl(
      FilePath(FILE_PATH_LITERAL("translate/es")),
      FilePath(FILE_PATH_LITERAL("google.html")));
  content::WindowedNotificationObserver language_observer1(
      chrome::NOTIFICATION_TAB_LANGUAGE_DETERMINED,
      content::NotificationService::AllSources());
  ui_test_utils::NavigateToURL(browser(), url);
  language_observer1.Wait();
  // Verify that the translate infobar showed up.
  EXPECT_EQ(1u, infobar_helper->GetInfoBarCount());
  InfoBarDelegate* infobar_delegate = infobar_helper->GetInfoBarDelegateAt(0);
  TranslateInfoBarDelegate* delegate =
      infobar_delegate->AsTranslateInfoBarDelegate();
  ASSERT_TRUE(delegate);
  EXPECT_EQ(TranslateInfoBarDelegate::BEFORE_TRANSLATE, delegate->type());
  EXPECT_EQ("es", delegate->GetOriginalLanguageCode());

  // Now force disable translate.
  ui_test_utils::CloseAllInfoBars(tab_contents);
  EXPECT_EQ(0u, infobar_helper->GetInfoBarCount());
  policies.Set(key::kTranslateEnabled, POLICY_LEVEL_MANDATORY,
               POLICY_SCOPE_USER, base::Value::CreateBooleanValue(false));
  provider_.UpdateChromePolicy(policies);
  // Navigating to the same URL now doesn't trigger an infobar.
  content::WindowedNotificationObserver language_observer2(
      chrome::NOTIFICATION_TAB_LANGUAGE_DETERMINED,
      content::NotificationService::AllSources());
  ui_test_utils::NavigateToURL(browser(), url);
  language_observer2.Wait();
  EXPECT_EQ(0u, infobar_helper->GetInfoBarCount());
}

}  // namespace policy
