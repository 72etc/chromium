// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_COMMON_LOCALIZED_ERROR_H_
#define CHROME_COMMON_LOCALIZED_ERROR_H_

#include <string>

#include "base/basictypes.h"
#include "base/string16.h"

class GURL;

namespace base {
class DictionaryValue;
}

namespace extensions {
class Extension;
}

namespace WebKit {
struct WebURLError;
}

class LocalizedError {
 public:
  // Fills |error_strings| with values to be used to build an error page used
  // on HTTP errors, like 404 or connection reset.
  static void GetStrings(const WebKit::WebURLError& error,
                         bool is_post,
                         const std::string& locale,
                         base::DictionaryValue* strings);

  // Returns a description of the encountered error.
  static string16 GetErrorDetails(const WebKit::WebURLError& error,
                                  bool is_post);

  // Returns true if an error page exists for the specified parameters.
  static bool HasStrings(const std::string& error_domain, int error_code);

  // Fills |error_strings| with values to be used to build an error page used
  // on HTTP errors, like 404 or connection reset, but using information from
  // the associated |app| in order to make the error page look like it's more
  // part of the app.
  static void GetAppErrorStrings(const WebKit::WebURLError& error,
                                 const GURL& display_url,
                                 const extensions::Extension* app,
                                 base::DictionaryValue* error_strings);

  static const char kHttpErrorDomain[];

 private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(LocalizedError);
};

#endif  // CHROME_COMMON_LOCALIZED_ERROR_H_
