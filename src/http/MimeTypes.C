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
  { "7z", "application/x-7z-compressed" },
  { "aac", "audio/aac" },
  { "abw", "application/x-abiword" },
  { "apng", "image/apng" },
  { "arc", "application/x-freearc" },
  { "avif", "image/avif" },
  { "avi", "video/x-msvideo" },
  { "azw", "application/vnd.amazon.ebook" },
  { "bin", "application/octet-stream" },
  { "bmp", "image/bmp" },
  { "bz", "application/x-bzip" },
  { "bz2", "application/x-bzip2" },
  { "cda", "application/x-cdf" },
  { "csh", "application/x-csh" },
  { "css", "text/css" },
  { "csv", "text/csv" },
  { "doc", "application/msword" },
  { "docx", "application/vnd.openxmlformats-officedocument.wordprocessingml.document" },
  { "eot", "application/vnd.ms-fontobject" },
  { "epub", "application/epub+zip" },
  { "gif", "image/gif" },
  { "gz", "application/gzip" },
  { "htm", "text/html" },
  { "html", "text/html" },
  { "ico", "image/vnd.microsoft.icon" },
  { "ics", "text/calendar" },
  { "jar", "application/java-archive" },
  { "jpg", "image/jpeg" },
  { "jpeg", "image/jpeg" },
  { "js", "text/javascript" },
  { "json", "application/json" },
  { "jsonld", "application/ld+json" },
  { "mid", "audio/x-midi" },
  { "midi", "audio/x-midi" },
  { "mjs", "text/javascript" },
  { "mp3", "audio/mp3" },
  { "mp4", "video/mp4" },
  { "mpeg", "video/mpeg" },
  { "mpkg", "application/vnd.apple.installer+xml" },
  { "mv4", "video/mp4" },
  { "odp", "application/vnd.oasis.opendocument.presentation" },
  { "ods", "application/vnd.oasis.opendocument.spreadsheet" },
  { "odt", "application/vnd.oasis.opendocument.text" },
  { "oga", "audio/ogg" },
  { "ogg", "audio/ogg" },
  { "ogv", "video/ogg" },
  { "ogx", "application/ogg" },
  { "opus", "audio/ogg" },
  { "otf", "font/otf" },
  { "png", "image/png" },
  { "pdf", "application/pdf" },
  { "php", "application/x-httpd-php" },
  { "ppt", "application/vnd.ms-powerpoint" },
  { "pptx", "application/vnd.openxmlformats-officedocument.presentationml.presentation" },
  { "rar", "application/vnd.rar" },
  { "rtf", "application/rtf" },
  { "sh", "application/x-sh" },
  { "svg", "image/svg+xml" },
  { "swf", "application/x-shockwave-flash" },
  { "tar", "application/x-tar" },
  { "tif", "image/tiff" },
  { "tiff", "image/tiff" },
  { "ts", "video/mp2t" },
  { "ttf", "font/ttf" },
  { "txt", "text/plain" },
  { "vsd", "application/vnd.visio" },
  { "wasm", "application/wasm" },
  { "wav", "audio/wav" },
  { "weba", "audio/webm" },
  { "webm", "video/webm" },
  { "webp", "image/webp" },
  { "woff", "font/woff" },
  { "woff2", "font/woff2" },
  { "xhtml", "application/xhtml+xml" },
  { "xls", "application/vnd.ms-excel" },
  { "xlsx", "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet" },
  { "xml", "application/xml" },
  { "xul", "application/vnd.mozilla.xul+xml" },
  { "zip", "application/zip" },
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
