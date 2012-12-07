// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_RENDERER_RENDER_WIDGET_FULLSCREEN_H_
#define CONTENT_RENDERER_RENDER_WIDGET_FULLSCREEN_H_

#include "content/renderer/render_widget.h"

#include "third_party/WebKit/Source/WebKit/chromium/public/WebWidget.h"

namespace content {

// TODO(boliu): Override non-supported methods with no-op? eg setWindowRect().
class RenderWidgetFullscreen : public RenderWidget {
 public:
  virtual void show(WebKit::WebNavigationPolicy);

 protected:
  RenderWidgetFullscreen(const WebKit::WebScreenInfo& screen_info);
  virtual ~RenderWidgetFullscreen();

  virtual WebKit::WebWidget* CreateWebWidget();

  bool Init(int32 opener_id);
};

}  // namespace content

#endif  // CONTENT_RENDERER_RENDER_WIDGET_FULLSCREEN_H_
