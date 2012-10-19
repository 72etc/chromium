// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ash/focus_cycler.h"

#include "ash/launcher/launcher.h"
#include "ash/shell.h"
#include "ash/shell_window_ids.h"
#include "ash/system/status_area_widget.h"
#include "ash/system/status_area_widget_delegate.h"
#include "ash/system/tray/system_tray.h"
#include "ash/wm/window_util.h"
#include "ash/test/ash_test_base.h"
#include "ash/shell_factory.h"
#include "ui/aura/test/test_windows.h"
#include "ui/aura/window.h"
#include "ui/views/controls/button/menu_button.h"
#include "ui/views/widget/widget.h"

namespace ash {
namespace test {

using aura::test::CreateTestWindowWithId;
using aura::Window;
using internal::FocusCycler;

namespace {

internal::StatusAreaWidgetDelegate* GetStatusAreaWidgetDelegate(
    views::Widget* widget) {
  return static_cast<internal::StatusAreaWidgetDelegate*>(
      widget->GetContentsView());
}

SystemTray* CreateSystemTray() {
  internal::StatusAreaWidget* widget = new internal::StatusAreaWidget;
  widget->CreateTrayViews(NULL);
  widget->Show();
  return widget->system_tray();
}

}  // namespace

typedef AshTestBase FocusCyclerTest;

// FocusCyclerTest and FocusCyclerLauncherTest tests are disabled due to
//  use-after-free bugs in them detectable by both AddressSanitizer and Valgrind.
// See also http://crbug.com/156827.
TEST_F(FocusCyclerTest, DISABLED_CycleFocusBrowserOnly) {
  scoped_ptr<FocusCycler> focus_cycler(new FocusCycler());

  // Create a single test window.
  scoped_ptr<Window> window0(CreateTestWindowWithId(0, NULL));
  wm::ActivateWindow(window0.get());
  EXPECT_TRUE(wm::IsActiveWindow(window0.get()));

  // Cycle the window
  focus_cycler->RotateFocus(FocusCycler::FORWARD);
  EXPECT_TRUE(wm::IsActiveWindow(window0.get()));
}

TEST_F(FocusCyclerTest, DISABLED_CycleFocusForward) {
  scoped_ptr<FocusCycler> focus_cycler(new FocusCycler());

  // Add the Status area
  scoped_ptr<SystemTray> tray(CreateSystemTray());
  ASSERT_TRUE(tray->GetWidget());
  focus_cycler->AddWidget(tray->GetWidget());
  GetStatusAreaWidgetDelegate(tray->GetWidget())->SetFocusCyclerForTesting(
      focus_cycler.get());

  // Add the launcher
  Launcher* launcher = Launcher::ForPrimaryDisplay();
  ASSERT_TRUE(launcher);
  views::Widget* launcher_widget = launcher->widget();
  ASSERT_TRUE(launcher_widget);
  launcher->SetFocusCycler(focus_cycler.get());

  // Create a single test window.
  scoped_ptr<Window> window0(CreateTestWindowWithId(0, NULL));
  wm::ActivateWindow(window0.get());
  EXPECT_TRUE(wm::IsActiveWindow(window0.get()));

  // Cycle focus to the status area
  focus_cycler->RotateFocus(FocusCycler::FORWARD);
  EXPECT_TRUE(tray->GetWidget()->IsActive());

  // Cycle focus to the launcher
  focus_cycler->RotateFocus(FocusCycler::FORWARD);
  EXPECT_TRUE(launcher_widget->IsActive());

  // Cycle focus to the browser
  focus_cycler->RotateFocus(FocusCycler::FORWARD);
  EXPECT_TRUE(wm::IsActiveWindow(window0.get()));
}

TEST_F(FocusCyclerTest, DISABLED_CycleFocusBackward) {
  scoped_ptr<FocusCycler> focus_cycler(new FocusCycler());

  // Add the Status area
  scoped_ptr<SystemTray> tray(CreateSystemTray());
  ASSERT_TRUE(tray->GetWidget());
  focus_cycler->AddWidget(tray->GetWidget());
  GetStatusAreaWidgetDelegate(tray->GetWidget())->SetFocusCyclerForTesting(
      focus_cycler.get());

  // Add the launcher
  Launcher* launcher = Launcher::ForPrimaryDisplay();
  ASSERT_TRUE(launcher);
  views::Widget* launcher_widget = launcher->widget();
  ASSERT_TRUE(launcher_widget);
  launcher->SetFocusCycler(focus_cycler.get());

  // Create a single test window.
  scoped_ptr<Window> window0(CreateTestWindowWithId(0, NULL));
  wm::ActivateWindow(window0.get());
  EXPECT_TRUE(wm::IsActiveWindow(window0.get()));

  // Cycle focus to the launcher
  focus_cycler->RotateFocus(FocusCycler::BACKWARD);
  EXPECT_TRUE(launcher_widget->IsActive());

  // Cycle focus to the status area
  focus_cycler->RotateFocus(FocusCycler::BACKWARD);
  EXPECT_TRUE(tray->GetWidget()->IsActive());

  // Cycle focus to the browser
  focus_cycler->RotateFocus(FocusCycler::BACKWARD);
  EXPECT_TRUE(wm::IsActiveWindow(window0.get()));
}

TEST_F(FocusCyclerTest, DISABLED_CycleFocusForwardBackward) {
  scoped_ptr<FocusCycler> focus_cycler(new FocusCycler());

  // Add the Status area
  scoped_ptr<SystemTray> tray(CreateSystemTray());
  ASSERT_TRUE(tray->GetWidget());
  focus_cycler->AddWidget(tray->GetWidget());
  GetStatusAreaWidgetDelegate(tray->GetWidget())->SetFocusCyclerForTesting(
      focus_cycler.get());

  // Add the launcher
  Launcher* launcher = Launcher::ForPrimaryDisplay();
  ASSERT_TRUE(launcher);
  views::Widget* launcher_widget = launcher->widget();
  ASSERT_TRUE(launcher_widget);
  launcher->SetFocusCycler(focus_cycler.get());

  // Create a single test window.
  scoped_ptr<Window> window0(CreateTestWindowWithId(0, NULL));
  wm::ActivateWindow(window0.get());
  EXPECT_TRUE(wm::IsActiveWindow(window0.get()));

  // Cycle focus to the launcher
  focus_cycler->RotateFocus(FocusCycler::BACKWARD);
  EXPECT_TRUE(launcher_widget->IsActive());

  // Cycle focus to the status area
  focus_cycler->RotateFocus(FocusCycler::BACKWARD);
  EXPECT_TRUE(tray->GetWidget()->IsActive());

  // Cycle focus to the browser
  focus_cycler->RotateFocus(FocusCycler::BACKWARD);
  EXPECT_TRUE(wm::IsActiveWindow(window0.get()));

  // Cycle focus to the status area
  focus_cycler->RotateFocus(FocusCycler::FORWARD);
  EXPECT_TRUE(tray->GetWidget()->IsActive());

  // Cycle focus to the launcher
  focus_cycler->RotateFocus(FocusCycler::FORWARD);
  EXPECT_TRUE(launcher_widget->IsActive());

  // Cycle focus to the browser
  focus_cycler->RotateFocus(FocusCycler::FORWARD);
  EXPECT_TRUE(wm::IsActiveWindow(window0.get()));
}

TEST_F(FocusCyclerTest, DISABLED_CycleFocusNoBrowser) {
  scoped_ptr<FocusCycler> focus_cycler(new FocusCycler());

  // Add the Status area
  scoped_ptr<SystemTray> tray(CreateSystemTray());
  ASSERT_TRUE(tray->GetWidget());
  focus_cycler->AddWidget(tray->GetWidget());
  GetStatusAreaWidgetDelegate(tray->GetWidget())->SetFocusCyclerForTesting(
      focus_cycler.get());

  // Add the launcher and focus it
  Launcher* launcher = Launcher::ForPrimaryDisplay();
  ASSERT_TRUE(launcher);
  views::Widget* launcher_widget = launcher->widget();
  ASSERT_TRUE(launcher_widget);
  launcher->SetFocusCycler(focus_cycler.get());
  focus_cycler->FocusWidget(launcher_widget);

  // Cycle focus to the status area
  focus_cycler->RotateFocus(FocusCycler::FORWARD);
  EXPECT_TRUE(tray->GetWidget()->IsActive());

  // Cycle focus to the launcher
  focus_cycler->RotateFocus(FocusCycler::FORWARD);
  EXPECT_TRUE(launcher_widget->IsActive());

  // Cycle focus to the status area
  focus_cycler->RotateFocus(FocusCycler::FORWARD);
  EXPECT_TRUE(tray->GetWidget()->IsActive());

  // Cycle focus to the launcher
  focus_cycler->RotateFocus(FocusCycler::BACKWARD);
  EXPECT_TRUE(launcher_widget->IsActive());

  // Cycle focus to the status area
  focus_cycler->RotateFocus(FocusCycler::BACKWARD);
  EXPECT_TRUE(tray->GetWidget()->IsActive());
}

class FocusCyclerLauncherTest : public AshTestBase {
 public:
  FocusCyclerLauncherTest() : AshTestBase() {}
  virtual ~FocusCyclerLauncherTest() {}

  virtual void SetUp() OVERRIDE {
    AshTestBase::SetUp();

    // Hide the launcher
    Launcher* launcher = Launcher::ForPrimaryDisplay();
    ASSERT_TRUE(launcher);
    views::Widget* launcher_widget = launcher->widget();
    ASSERT_TRUE(launcher_widget);
    launcher_widget->Hide();
  }

  virtual void TearDown() OVERRIDE {
    // Show the launcher
    Launcher* launcher = Launcher::ForPrimaryDisplay();
    ASSERT_TRUE(launcher);
    views::Widget* launcher_widget = launcher->widget();
    ASSERT_TRUE(launcher_widget);
    launcher_widget->Show();

    AshTestBase::TearDown();
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(FocusCyclerLauncherTest);
};

TEST_F(FocusCyclerLauncherTest, DISABLED_CycleFocusForwardInvisible) {
  scoped_ptr<FocusCycler> focus_cycler(new FocusCycler());

  // Add the Status area
  scoped_ptr<SystemTray> tray(CreateSystemTray());
  ASSERT_TRUE(tray->GetWidget());
  focus_cycler->AddWidget(tray->GetWidget());
  GetStatusAreaWidgetDelegate(tray->GetWidget())->SetFocusCyclerForTesting(
      focus_cycler.get());

  // Add the launcher
  Launcher* launcher = Launcher::ForPrimaryDisplay();
  ASSERT_TRUE(launcher);
  views::Widget* launcher_widget = launcher->widget();
  ASSERT_TRUE(launcher_widget);
  launcher->SetFocusCycler(focus_cycler.get());

  // Create a single test window.
  scoped_ptr<Window> window0(CreateTestWindowWithId(0, NULL));
  wm::ActivateWindow(window0.get());
  EXPECT_TRUE(wm::IsActiveWindow(window0.get()));

  // Cycle focus to the status area
  focus_cycler->RotateFocus(FocusCycler::FORWARD);
  EXPECT_TRUE(tray->GetWidget()->IsActive());

  // Cycle focus to the browser
  focus_cycler->RotateFocus(FocusCycler::FORWARD);
  EXPECT_TRUE(wm::IsActiveWindow(window0.get()));
}

TEST_F(FocusCyclerLauncherTest, DISABLED_CycleFocusBackwardInvisible) {
  scoped_ptr<FocusCycler> focus_cycler(new FocusCycler());

  // Add the Status area
  scoped_ptr<SystemTray> tray(CreateSystemTray());
  ASSERT_TRUE(tray->GetWidget());
  focus_cycler->AddWidget(tray->GetWidget());
  GetStatusAreaWidgetDelegate(tray->GetWidget())->SetFocusCyclerForTesting(
      focus_cycler.get());

  // Add the launcher
  Launcher* launcher = Launcher::ForPrimaryDisplay();
  ASSERT_TRUE(launcher);
  views::Widget* launcher_widget = launcher->widget();
  ASSERT_TRUE(launcher_widget);
  launcher->SetFocusCycler(focus_cycler.get());

  // Create a single test window.
  scoped_ptr<Window> window0(CreateTestWindowWithId(0, NULL));
  wm::ActivateWindow(window0.get());
  EXPECT_TRUE(wm::IsActiveWindow(window0.get()));

  // Cycle focus to the status area
  focus_cycler->RotateFocus(FocusCycler::BACKWARD);
  EXPECT_TRUE(tray->GetWidget()->IsActive());

  // Cycle focus to the browser
  focus_cycler->RotateFocus(FocusCycler::BACKWARD);
  EXPECT_TRUE(wm::IsActiveWindow(window0.get()));
}

}  // namespace test
}  // namespace ash
