// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_CHROMEOS_GDATA_GDATA_ERRORCODE_H_
#define CHROME_BROWSER_CHROMEOS_GDATA_GDATA_ERRORCODE_H_

namespace gdata {

// HTTP errors that can be returned by GData service.
enum GDataErrorCode {
  HTTP_SUCCESS               = 200,
  HTTP_CREATED               = 201,
  HTTP_FOUND                 = 302,
  HTTP_NOT_MODIFIED          = 304,
  HTTP_RESUME_INCOMPLETE     = 308,
  HTTP_BAD_REQUEST           = 400,
  HTTP_UNAUTHORIZED          = 401,
  HTTP_FORBIDDEN             = 403,
  HTTP_NOT_FOUND             = 404,
  HTTP_CONFLICT              = 409,
  HTTP_LENGTH_REQUIRED       = 411,
  HTTP_PRECONDITION          = 412,
  HTTP_INTERNAL_SERVER_ERROR = 500,
  HTTP_SERVICE_UNAVAILABLE   = 503,
  GDATA_PARSE_ERROR          = -100,
  GDATA_FILE_ERROR           = -101,
  GDATA_CANCELLED            = -102,
  GDATA_OTHER_ERROR          = -103,
  GDATA_NO_CONNECTION        = -104,
};

enum GDataFileError {
  GDATA_FILE_OK = 0,
  GDATA_FILE_ERROR_FAILED = -1,
  GDATA_FILE_ERROR_IN_USE = -2,
  GDATA_FILE_ERROR_EXISTS = -3,
  GDATA_FILE_ERROR_NOT_FOUND = -4,
  GDATA_FILE_ERROR_ACCESS_DENIED = -5,
  GDATA_FILE_ERROR_TOO_MANY_OPENED = -6,
  GDATA_FILE_ERROR_NO_MEMORY = -7,
  GDATA_FILE_ERROR_NO_SPACE = -8,
  GDATA_FILE_ERROR_NOT_A_DIRECTORY = -9,
  GDATA_FILE_ERROR_INVALID_OPERATION = -10,
  GDATA_FILE_ERROR_SECURITY = -11,
  GDATA_FILE_ERROR_ABORT = -12,
  GDATA_FILE_ERROR_NOT_A_FILE = -13,
  GDATA_FILE_ERROR_NOT_EMPTY = -14,
  GDATA_FILE_ERROR_INVALID_URL = -15,
};

}  // namespace gdata

#endif  // CHROME_BROWSER_CHROMEOS_GDATA_GDATA_ERRORCODE_H_
