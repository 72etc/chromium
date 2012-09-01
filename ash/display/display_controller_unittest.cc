// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ash/display/display_controller.h"

#include "ash/shell.h"
#include "ash/test/ash_test_base.h"
#include "ui/aura/display_manager.h"
#include "ui/aura/env.h"
#include "ui/aura/root_window.h"
#include "ui/gfx/display.h"
#include "ui/gfx/screen.h"

namespace ash {
namespace test {
namespace {

gfx::Display GetPrimaryDisplay() {
  return gfx::Screen::GetDisplayNearestWindow(
      Shell::GetAllRootWindows()[0]);
}

gfx::Display GetSecondaryDisplay() {
  return gfx::Screen::GetDisplayNearestWindow(
      Shell::GetAllRootWindows()[1]);
}

}  // namespace

typedef test::AshTestBase DisplayControllerTest;

#if defined(OS_WIN)
// TOD(oshima): Windows creates a window with smaller client area.
// Fix this and enable tests.
#define MAYBE_SecondaryDisplayLayout DISABLED_SecondaryDisplayLayout
#else
#define MAYBE_SecondaryDisplayLayout SecondaryDisplayLayout
#endif

TEST_F(DisplayControllerTest, MAYBE_SecondaryDisplayLayout) {
  UpdateDisplay("500x500,400x400");
  gfx::Display* secondary_display =
      aura::Env::GetInstance()->display_manager()->GetDisplayAt(1);
  gfx::Insets insets(5, 5, 5, 5);
  secondary_display->UpdateWorkAreaFromInsets(insets);

  // Default layout is LEFT.
  EXPECT_EQ("0,0 500x500", GetPrimaryDisplay().bounds().ToString());
  EXPECT_EQ("500,0 400x400", GetSecondaryDisplay().bounds().ToString());
  EXPECT_EQ("505,5 390x390", GetSecondaryDisplay().work_area().ToString());

  // Layout the secondary display to the bottom of the primary.
  Shell::GetInstance()->display_controller()->SetSecondaryDisplayLayout(
      internal::DisplayController::BOTTOM);
  EXPECT_EQ("0,0 500x500", GetPrimaryDisplay().bounds().ToString());
  EXPECT_EQ("0,500 400x400", GetSecondaryDisplay().bounds().ToString());
  EXPECT_EQ("5,505 390x390", GetSecondaryDisplay().work_area().ToString());

  // Layout the secondary display to the left of the primary.
  Shell::GetInstance()->display_controller()->SetSecondaryDisplayLayout(
      internal::DisplayController::LEFT);
  EXPECT_EQ("0,0 500x500", GetPrimaryDisplay().bounds().ToString());
  EXPECT_EQ("-400,0 400x400", GetSecondaryDisplay().bounds().ToString());
  EXPECT_EQ("-395,5 390x390", GetSecondaryDisplay().work_area().ToString());

  // Layout the secondary display to the top of the primary.
  Shell::GetInstance()->display_controller()->SetSecondaryDisplayLayout(
      internal::DisplayController::TOP);
  EXPECT_EQ("0,0 500x500", GetPrimaryDisplay().bounds().ToString());
  EXPECT_EQ("0,-400 400x400", GetSecondaryDisplay().bounds().ToString());
  EXPECT_EQ("5,-395 390x390", GetSecondaryDisplay().work_area().ToString());
}

// TODO(oshima,erg): I suspect this test is now failing because I've changed
// the timing of the RootWindow::Show to be synchronous. If true, this test has
// always been incorrect, but is now visibly broken now that we're processing
// X11 configuration events while waiting for the MapNotify.
TEST_F(DisplayControllerTest, DISABLED_BoundsUpdated) {
  Shell::GetInstance()->display_controller()->SetSecondaryDisplayLayout(
      internal::DisplayController::BOTTOM);
  UpdateDisplay("500x500,400x400");
  gfx::Display* secondary_display =
      aura::Env::GetInstance()->display_manager()->GetDisplayAt(1);
  gfx::Insets insets(5, 5, 5, 5);
  secondary_display->UpdateWorkAreaFromInsets(insets);

  EXPECT_EQ("0,0 500x500", GetPrimaryDisplay().bounds().ToString());
  EXPECT_EQ("0,500 400x400", GetSecondaryDisplay().bounds().ToString());
  EXPECT_EQ("5,505 390x390", GetSecondaryDisplay().work_area().ToString());

  UpdateDisplay("600x600,400x400");
  EXPECT_EQ("0,0 600x600", GetPrimaryDisplay().bounds().ToString());
  EXPECT_EQ("0,600 400x400", GetSecondaryDisplay().bounds().ToString());
  EXPECT_EQ("5,605 390x390", GetSecondaryDisplay().work_area().ToString());

  UpdateDisplay("600x600,500x500");
  EXPECT_EQ("0,0 600x600", GetPrimaryDisplay().bounds().ToString());
  EXPECT_EQ("0,600 500x500", GetSecondaryDisplay().bounds().ToString());
  EXPECT_EQ("5,605 490x490", GetSecondaryDisplay().work_area().ToString());

  UpdateDisplay("600x600");
  EXPECT_EQ("0,0 600x600", GetPrimaryDisplay().bounds().ToString());
  EXPECT_EQ(1, gfx::Screen::GetNumDisplays());

  UpdateDisplay("700x700,1000x1000");
  ASSERT_EQ(2, gfx::Screen::GetNumDisplays());
  EXPECT_EQ("0,0 700x700", GetPrimaryDisplay().bounds().ToString());
  EXPECT_EQ("0,700 1000x1000", GetSecondaryDisplay().bounds().ToString());
}

}  // namespace test
}  // namespace ash
