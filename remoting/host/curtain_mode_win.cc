// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "remoting/host/curtain_mode.h"

#include "base/logging.h"
#include "base/single_thread_task_runner.h"
#include "remoting/host/client_session_control.h"

namespace remoting {

class CurtainModeWin : public CurtainMode {
 public:
  CurtainModeWin();

  // Overriden from CurtainMode.
  virtual bool Activate() OVERRIDE;

 private:
  DISALLOW_COPY_AND_ASSIGN(CurtainModeWin);
};

CurtainModeWin::CurtainModeWin() {
}

bool CurtainModeWin::Activate() {
  DWORD session_id;
  if (!ProcessIdToSessionId(GetCurrentProcessId(), &session_id)) {
    LOG_GETLASTERROR(ERROR) << "Failed to map the current PID to session ID";
    return false;
  }

  // The current session is curtained if it is not attached to the local
  // console.
  return WTSGetActiveConsoleSessionId() != session_id;
}

// static
scoped_ptr<CurtainMode> CurtainMode::Create(
    scoped_refptr<base::SingleThreadTaskRunner> caller_task_runner,
    scoped_refptr<base::SingleThreadTaskRunner> ui_task_runner,
    base::WeakPtr<ClientSessionControl> client_session_control) {
  // |client_session_control| is not used because the client session is
  // disconnected as soon as the session is re-attached to the local console.
  // See RdpDesktopSession for more details.
  return scoped_ptr<CurtainMode>(new CurtainModeWin());
}

}  // namespace remoting
