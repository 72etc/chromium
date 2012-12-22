// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_CONSTRAINED_WINDOW_H_
#define CHROME_BROWSER_UI_CONSTRAINED_WINDOW_H_

#include "build/build_config.h"
#include "ui/gfx/native_widget_types.h"

///////////////////////////////////////////////////////////////////////////////
// ConstrainedWindow
//
//  This interface represents a window that is constrained to a
//  WebContentsView's bounds.
//
class ConstrainedWindow {
 public:
  // Makes the web contents modal dialog visible. Only one web contents modal
  // dialog is shown at a time per tab.
  virtual void ShowWebContentsModalDialog() = 0;

  // Closes the web contents modal dialog.
  virtual void CloseWebContentsModalDialog() = 0;

  // Sets focus on the web contents modal dialog.
  virtual void FocusWebContentsModalDialog();

  // Runs a pulse animation for the web contents modal dialog.
  virtual void PulseWebContentsModalDialog();

  // Checks if the web contents modal dialog can be shown.
  virtual bool CanShowWebContentsModalDialog();

  // Returns the native window of the web contents modal dialog.
  virtual gfx::NativeWindow GetNativeWindow();

 protected:
  virtual ~ConstrainedWindow() {}
};

#endif  // CHROME_BROWSER_UI_CONSTRAINED_WINDOW_H_
