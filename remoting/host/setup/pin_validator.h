// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef REMOTING_HOST_SETUP_PIN_VALIDATOR_H_
#define REMOTING_HOST_SETUP_PIN_VALIDATOR_H_

#include <string>

namespace remoting {

// Returns whether a PIN is valid.
bool IsPinValid(const std::string& pin);

}  // namespace remoting

#endif  // REMOTING_HOST_SETUP_PIN_VALIDATOR_H_
