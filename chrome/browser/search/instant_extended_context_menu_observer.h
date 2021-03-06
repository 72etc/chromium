// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_SEARCH_INSTANT_EXTENDED_CONTEXT_MENU_OBSERVER_H_
#define CHROME_BROWSER_SEARCH_INSTANT_EXTENDED_CONTEXT_MENU_OBSERVER_H_

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "chrome/browser/tab_contents/render_view_context_menu_observer.h"
#include "googleurl/src/gurl.h"

namespace content {
class WebContents;
}

// This class disables menu items which perform poorly in instant extended mode.
class InstantExtendedContextMenuObserver
    : public RenderViewContextMenuObserver {
 public:
  InstantExtendedContextMenuObserver(content::WebContents* contents, GURL url);
  virtual ~InstantExtendedContextMenuObserver();

  // RenderViewContextMenuObserver implementation.
  virtual bool IsCommandIdSupported(int command_id) OVERRIDE;
  virtual bool IsCommandIdEnabled(int command_id) OVERRIDE;

 private:
  bool IsInstantOverlay();
  bool IsLocalPage();

  // The source web contents of the context menu.
  content::WebContents* contents_;

  // The page URL.
  GURL url_;

  DISALLOW_COPY_AND_ASSIGN(InstantExtendedContextMenuObserver);
};

#endif  // CHROME_BROWSER_SEARCH_INSTANT_EXTENDED_CONTEXT_MENU_OBSERVER_H_
