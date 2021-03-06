// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_RENDERER_PEPPER_PEPPER_FLASH_MENU_HOST_H_
#define CHROME_RENDERER_PEPPER_PEPPER_FLASH_MENU_HOST_H_

#include <vector>

#include "base/compiler_specific.h"
#include "content/public/renderer/context_menu_client.h"
#include "ppapi/c/pp_point.h"
#include "ppapi/host/host_message_context.h"
#include "ppapi/host/resource_host.h"

struct WebMenuItem;

namespace content {
class RendererPpapiHost;
}

namespace ppapi {
namespace proxy {
class SerializedFlashMenu;
}
}

namespace chrome {

class PepperFlashMenuHost : public ppapi::host::ResourceHost,
                            public content::ContextMenuClient {
 public:
  PepperFlashMenuHost(content::RendererPpapiHost* host,
                      PP_Instance instance,
                      PP_Resource resource,
                      const ppapi::proxy::SerializedFlashMenu& serial_menu);
  virtual ~PepperFlashMenuHost();

  virtual int32_t OnResourceMessageReceived(
      const IPC::Message& msg,
      ppapi::host::HostMessageContext* context) OVERRIDE;

 private:
  int32_t OnHostMsgShow(ppapi::host::HostMessageContext* context,
                        const PP_Point& location);

  // ContextMenuClient implementation.
  virtual void OnMenuAction(int request_id, unsigned action) OVERRIDE;
  virtual void OnMenuClosed(int request_id) OVERRIDE;

  void SendMenuReply(int32_t result, int action);

  content::RendererPpapiHost* renderer_ppapi_host_;

  bool showing_context_menu_;
  int context_menu_request_id_;

  std::vector<WebMenuItem> menu_data_;

  // We send |WebMenuItem|s, which have an |unsigned| "action" field instead of
  // an |int32_t| ID. (CONTENT also limits the range of valid values for
  // actions.) This maps actions to IDs.
  std::vector<int32_t> menu_id_map_;

  // Used to send a single context menu "completion" upon menu close.
  bool has_saved_context_menu_action_;
  unsigned saved_context_menu_action_;

  DISALLOW_COPY_AND_ASSIGN(PepperFlashMenuHost);
};

}  // namespace chrome

#endif  // CHROME_RENDERER_PEPPER_PEPPER_FLASH_MENU_HOST_H_
