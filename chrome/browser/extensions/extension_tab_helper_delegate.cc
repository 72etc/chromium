// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/extensions/extension_tab_helper_delegate.h"

ExtensionTabHelperDelegate::~ExtensionTabHelperDelegate() {
}

// Notification when an application programmatically requests installation.
void ExtensionTabHelperDelegate::OnInstallApplication(
    TabContents* source, const WebApplicationInfo& app_info) {
}

void ExtensionTabHelperDelegate::OnDidGetApplicationInfo(
    TabContents* source, int32 page_id) {
}
