/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * All rights reserved.
 */
//
// mime_types.cpp
// ~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2006 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "MimeTypes.h"

#include <boost/algorithm/string/predicate.hpp>

namespace http {
namespace server {
namespace mime_types {

struct mapping
{
  const char *extension;
  const char * mime_type;
} mappings[] =
{
  { "css", "text/css" },
  { "gif", "image/gif" },
  { "htm", "text/html" },
  { "html", "text/html" },
  { "jpg", "image/jpeg" },
  { "png", "image/png" },
  { "js", "text/javascript" },
  { "wasm", "application/wasm" },
  { "oga", "audio/ogg" },
  { "ogg", "audio/ogg" },
  { "ogv", "video/ogg" },
  { "swf", "application/x-shockwave-flash" },
  { "mp4", "video/mp4" },
  { "mv4", "video/mp4" },
  { "mp3", "audio/mp3" },
  { "svg", "image/svg+xml" },
  { "webm", "video/webm" },
  { "xml", "application/xml" },
  { "pdf", "application/pdf" },
  { 0, 0 } // Marks end of list.
};

const char *extensionToType(const std::string& extension)
{
  for (mapping* m = mappings; m->extension; ++m) {
    if (boost::iequals(m->extension, extension)) {
      return m->mime_type;
    }
  }

  return "application/octet-stream";
}

} // namespace mime_types
} // namespace server
} // namespace http
