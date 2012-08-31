// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_VIEWS_FRAME_APP_NON_CLIENT_FRAME_VIEW_AURA_H_
#define CHROME_BROWSER_UI_VIEWS_FRAME_APP_NON_CLIENT_FRAME_VIEW_AURA_H_

#include "chrome/browser/ui/views/frame/browser_non_client_frame_view.h"
#include "ui/views/controls/button/button.h"

namespace aura {
class Window;
}

// NonClientFrameViewAura implementation for apps.
class AppNonClientFrameViewAura : public BrowserNonClientFrameView {
 public:
  AppNonClientFrameViewAura(
      BrowserFrame* frame, BrowserView* browser_view);
  virtual ~AppNonClientFrameViewAura();

  // NonClientFrameView:
  virtual gfx::Rect GetBoundsForClientView() const OVERRIDE;
  virtual gfx::Rect GetWindowBoundsForClientBounds(
      const gfx::Rect& client_bounds) const OVERRIDE;
  virtual int NonClientHitTest(const gfx::Point& point) OVERRIDE;
  virtual void GetWindowMask(
      const gfx::Size& size,
      gfx::Path* window_mask) OVERRIDE;
  virtual void ResetWindowControls() OVERRIDE;
  virtual void UpdateWindowIcon() OVERRIDE;

  // BrowserNonClientFrameView:
  virtual gfx::Rect GetBoundsForTabStrip(
      views::View* tabstrip) const OVERRIDE;
  virtual TabStripInsets GetTabStripInsets(bool restored) const OVERRIDE;
  virtual int GetThemeBackgroundXInset() const OVERRIDE;
  virtual void UpdateThrobber(bool running) OVERRIDE;

  // View:
  virtual void OnBoundsChanged(const gfx::Rect& previous_bounds) OVERRIDE;

  // Close the app window.
  void Close();

  // Restore the app window (will result in switching the frame_view back).
  void Restore();

 private:
  class ControlView;

  gfx::Rect GetControlBounds() const;

  // The View containing the restore and close buttons.
  ControlView* control_view_;
  // The widget holding the control_view_.
  views::Widget* control_widget_;

  DISALLOW_COPY_AND_ASSIGN(AppNonClientFrameViewAura);
};

#endif  // CHROME_BROWSER_UI_VIEWS_FRAME_APP_NON_CLIENT_FRAME_VIEW_AURA_H_
