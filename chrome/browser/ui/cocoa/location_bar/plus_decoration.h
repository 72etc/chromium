// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_COCOA_LOCATION_BAR_PLUS_DECORATION_H_
#define CHROME_BROWSER_UI_COCOA_LOCATION_BAR_PLUS_DECORATION_H_

#import <Cocoa/Cocoa.h>

#include "chrome/browser/ui/cocoa/location_bar/image_decoration.h"
#include "chrome/browser/ui/toolbar/action_box_button_controller.h"

class Browser;
class LocationBarViewMac;

// Note: this file is under development (see crbug.com/138118).

// Plus icon on the right side of the location bar.
class PlusDecoration : public ImageDecoration,
                       public ActionBoxButtonController::Delegate {
 public:
  PlusDecoration(LocationBarViewMac* owner, Browser* browser);
  virtual ~PlusDecoration();

  // Helper to get where the action box menu and bubble point should be
  // anchored. Similar to |PageActionDecoration| or |StarDecoration|.
  NSPoint GetActionBoxAnchorPoint();

  // Implement |LocationBarDecoration|.
  virtual bool AcceptsMousePress() OVERRIDE;
  virtual bool OnMousePressed(NSRect frame) OVERRIDE;
  virtual NSString* GetToolTip() OVERRIDE;

  ActionBoxButtonController* action_box_button_controller() {
    return &controller_;
  }

 private:
  // Implement |ActionBoxButtonController::Delegate|.
  virtual void ShowMenu(scoped_ptr<ActionBoxMenuModel> model) OVERRIDE;

  // Owner of the decoration, used to obtain the menu.
  LocationBarViewMac* owner_;

  Browser* browser_;

  ActionBoxButtonController controller_;

  DISALLOW_COPY_AND_ASSIGN(PlusDecoration);
};

#endif  // CHROME_BROWSER_UI_COCOA_LOCATION_BAR_PLUS_DECORATION_H_
