// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_BROWSER_BROWSER_PLUGIN_BROWSER_PLUGIN_EMBEDDER_HELPER_H_
#define CONTENT_BROWSER_BROWSER_PLUGIN_BROWSER_PLUGIN_EMBEDDER_HELPER_H_

#include <string>

#include "base/compiler_specific.h"
#include "content/public/browser/render_view_host_observer.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebDragStatus.h"
#include "third_party/WebKit/Source/WebKit/chromium/public/WebDragOperation.h"

namespace IPC {
class Message;
class SyncMessage;
}

namespace gfx {
class Point;
class Size;
}

struct BrowserPluginHostMsg_ResizeGuest_Params;
struct WebDropData;

namespace content {

class BrowserPluginEmbedder;
class RenderViewHost;

// Helper for browser plugin embedder.
//
// A lot of messages coming from guests need to know the guest's RenderViewHost.
// BrowserPluginEmbedderHelper handles BrowserPluginHostMsg messages and
// relays them with their associated RenderViewHosts to its delegate where they
// will be handled.
//
// A BrowserPluginEmbedderHelper is created whenever a BrowserPluginEmbedder is
// created. BrowserPluginEmbedder's lifetime is managed by the associated
// RenderViewHost. Functions in this class is assumed to be run on UI thread.
class BrowserPluginEmbedderHelper : public RenderViewHostObserver {
 public:
  BrowserPluginEmbedderHelper(BrowserPluginEmbedder* embedder,
                              RenderViewHost* render_view_host);
  virtual ~BrowserPluginEmbedderHelper();

  // Make it public for sync IPCs.
  virtual bool Send(IPC::Message* message) OVERRIDE;

 protected:
  // RenderViewHostObserver implementation.
  virtual bool OnMessageReceived(const IPC::Message& message) OVERRIDE;

 private:
  // Message handlers.
  void OnCreateGuest(int instance_id,
                     const std::string& storage_partition_id,
                     bool persist_storage);
  void OnNavigateGuest(int instance_id,
                       const std::string& src,
                       const BrowserPluginHostMsg_ResizeGuest_Params& params);
  void OnResizeGuest(int instance_id,
                     const BrowserPluginHostMsg_ResizeGuest_Params& params);
  void OnUpdateRectACK(int instance_id, int message_id, const gfx::Size& size);
  void OnHandleInputEvent(const IPC::SyncMessage& message, bool* handled);
  void OnSetFocus(int instance_id, bool focused);
  void OnPluginDestroyed(int instance_id);
  void OnGo(int instance_id, int relative_index);
  void OnStop(int instance_id);
  void OnReload(int instance_id);
  void OnTerminateGuest(int instance_id);
  void OnSetGuestVisibility(int instance_id, bool visible);
  void OnDragStatusUpdate(int instance_id,
                          WebKit::WebDragStatus drag_status,
                          const WebDropData& drop_data,
                          WebKit::WebDragOperationsMask drag_mask,
                          const gfx::Point& location);

  BrowserPluginEmbedder* embedder_;

  DISALLOW_COPY_AND_ASSIGN(BrowserPluginEmbedderHelper);
};

}  // namespace content

#endif  // CONTENT_BROWSER_BROWSER_PLUGIN_BROWSER_PLUGIN_EMBEDDER_HELPER_H_
