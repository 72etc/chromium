// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_RENDERER_DISAMBIGUATION_POPUP_HELPER_H_
#define CONTENT_RENDERER_DISAMBIGUATION_POPUP_HELPER_H_

#include "content/common/content_export.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/platform/WebVector.h"

namespace gfx {
class Rect;
class Size;
}

namespace WebKit {
struct WebRect;
}

namespace content {

// Contains functions to calculate proper scaling factor and popup size
class DisambiguationPopupHelper {
 public:
  CONTENT_EXPORT static float ComputeZoomAreaAndScaleFactor(
      const gfx::Rect& tap_rect,
      const WebKit::WebVector<WebKit::WebRect>& target_rects,
      const gfx::Size& viewport_size,
      gfx::Rect* zoom_rect);
};

}  // namespace content

#endif  // CONTENT_RENDERER_DISAMBIGUATION_POPUP_HELPER_H_
