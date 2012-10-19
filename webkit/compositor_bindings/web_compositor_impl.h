// Copyright 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WebCompositorImpl_h
#define WebCompositorImpl_h

#include "third_party/WebKit/Source/Platform/chromium/public/WebCompositor.h"

namespace cc {
class CCThread;
}

namespace WebKit {

class WebThread;

class WebCompositorImpl : public WebCompositor {
public:
    static bool initialized();

    static void initialize(WebThread* implThread);
    static bool isThreadingEnabled();
    static void shutdown();

private:
    static bool s_initialized;
    static cc::CCThread* s_mainThread;
    static cc::CCThread* s_implThread;
};

}

#endif // WebCompositorImpl_h
