// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WEBKIT_PLUGINS_PPAPI_V8_VAR_CONVERTER_H
#define WEBKIT_PLUGINS_PPAPI_V8_VAR_CONVERTER_H


#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "ppapi/c/pp_var.h"
#include "v8/include/v8.h"
#include "webkit/plugins/webkit_plugins_export.h"

namespace webkit {
namespace ppapi {

// Class to convert between PP_Vars and V8 values.
class WEBKIT_PLUGINS_EXPORT V8VarConverter {
 public:
  V8VarConverter();

  // Converts the given PP_Var to a v8::Value. True is returned upon success.
  bool ToV8Value(const PP_Var& var,
                 v8::Handle<v8::Context> context,
                 v8::Handle<v8::Value>* result) const;
  // Converts the given v8::Value to a PP_Var. True is returned upon success.
  // Every PP_Var in the reference graph of which |result| is apart will have
  // a refcount equal to the number of references to it in the graph. |result|
  // will have one additional reference.
  bool FromV8Value(v8::Handle<v8::Value> val,
                   v8::Handle<v8::Context> context,
                   PP_Var* result) const;

  DISALLOW_COPY_AND_ASSIGN(V8VarConverter);
};

}  // namespace ppapi
}  // namespace webkit

#endif  // WEBKIT_PLUGINS_PPAPI_V8_VAR_CONVERTER_H
