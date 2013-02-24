// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/icon_manager.h"

#include "base/files/file_path.h"
#include "base/string_util.h"

IconGroupID IconManager::GetGroupIDFromFilepath(
    const base::FilePath& filepath) {
  return StringToLowerASCII(filepath.Extension());
}
