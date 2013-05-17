// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Determines whether a certain gpu is prefered in a dual-gpu situation.
// A valid gpu_switching_list.json file are in the format of
// {
//   "version": "x.y",
//   "entries": [
//     { // entry 1
//     },
//     ...
//     { // entry n
//     }
//   ]
// }
//
// Each entry contains the following fields (fields are optional unless
// specifically described as mandatory below):
// 1. "id" is an integer.  0 is reserved.  This field is mandatory.
// 2. "os" contains "type" and an optional "version". "type" could be "macosx",
//    "linux", "win", "chromeos", or "any".  "any" is the same as not specifying
//    "os".
//    "version" is a VERSION structure (defined below).
// 3. "vendor_id" is a string.  0 is reserved.
// 4. "device_id" is an array of strings.  0 is reserved.
// 5. "multi_gpu_style" is a string, valid values include "optimus", and
//    "amd_switchable".
// 6. "multi_gpu_category" is a string, valid values include "any", "primary",
//    and "secondary".  If unspecified, the default value is "primary".
// 7. "driver_vendor" is a STRING structure (defined below).
// 8. "driver_version" is a VERSION structure (defined below).
// 9. "driver_date" is a VERSION structure (defined below).
//    The version is interpreted as "year.month.day".
// 10. "gl_vendor" is a STRING structure (defined below).
// 11. "gl_renderer" is a STRING structure (defined below).
// 12. "gl_extensions" is a STRING structure (defined below).
// 13. "perf_graphics" is a FLOAT structure (defined below).
// 14. "perf_gaming" is a FLOAT structure (defined below).
// 15. "perf_overall" is a FLOAT structure (defined below).
// 16. "machine_model" contais "name" and an optional "version".  "name" is a
//     STRING structure and "version" is a VERSION structure (defined below).
// 17. "gpu_count" is a INT structure (defined below).
// 18  "cpu_info" is a STRING structure (defined below).
// 19. "exceptions" is a list of entries.
// 20. "features" is a list of gpu switching options, including
//     "force_discrete" and "force_integrated".
//     This field is mandatory.
// 21. "description" has the description of the entry.
// 22. "webkit_bugs" is an array of associated webkit bug numbers.
// 23. "cr_bugs" is an array of associated webkit bug numbers.
// 24. "browser_version" is a VERSION structure (defined below).  If this
//     condition is not satisfied, the entry will be ignored.  If it is not
//     present, then the entry applies to all versions of the browser.
// 25. "disabled" is a boolean. If it is present, the entry will be skipped.
//     This can not be used in exceptions.
//
// VERSION includes "op", "style", "number", and "number2".  "op" can be any of
// the following values: "=", "<", "<=", ">", ">=", "any", "between".  "style"
// is optional and can be "lexical" or "numerical"; if it's not specified, it
// defaults to "numerical".  "number2" is only used if "op" is "between".
// "between" is "number <= * <= number2".
// "number" is used for all "op" values except "any". "number" and "number2"
// are in the format of x, x.x, x.x.x, etc.
// Only "driver_version" supports lexical style if the format is major.minor;
// in that case, major is still numerical, but minor is lexical. 
//
// STRING includes "op" and "value".  "op" can be any of the following values:
// "contains", "beginwith", "endwith", "=".  "value" is a string.
//
// FLOAT includes "op" "value", and "value2".  "op" can be any of the
// following values: "=", "<", "<=", ">", ">=", "any", "between".  "value2" is
// only used if "op" is "between".  "value" is used for all "op" values except
// "any". "value" and "value2" are valid float numbers.
// INT is very much like FLOAT, except that the values need to be integers.

#include "content/browser/gpu/gpu_control_list_jsons.h"

#define LONG_STRING_CONST(...) #__VA_ARGS__

namespace content {

const char kGpuSwitchingListJson[] = LONG_STRING_CONST(

{
  "name": "gpu switching list",
  // Please update the version number whenever you change this file.
  "version": "2.0",
  "entries": [
    {
      "id": 1,
      "description": "Force to use discrete GPU on older MacBookPro models.",
      "cr_bugs": [113703],
      "os": {
        "type": "macosx",
        "version": {
          "op": ">=",
          "number": "10.7"
        }
      },
      "machine_model": {
        "name": {
          "op": "=",
          "value": "MacBookPro"
        },
        "version": {
          "op": "<",
          "number": "8"
        }
      },
      "gpu_count": {
        "op": "=",
        "value": "2"
      },
      "features": [
        "force_discrete"
      ]
    }
  ]
}

);  // LONG_STRING_CONST macro

}  // namespace content

