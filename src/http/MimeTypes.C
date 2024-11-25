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
#include "Wt/WLogger.h"
#include "web/WebUtils.h"

#include <fstream>
#include <map>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>

namespace Wt {
  LOGGER("wthttp/MimeTypes");
}

namespace http {
namespace server {
namespace mime_types {
using ExtensionMap = std::map<std::string, std::string>;

struct mapping
{
  std::string extension;
  std::string mime_type;
};

ExtensionMap defaultExtensionMappings()
{
  ExtensionMap defaultExtMap;
  defaultExtMap["7z"] = "application/x-7z-compressed";
  defaultExtMap["aac"] = "audio/aac";
  defaultExtMap["abw"] = "application/x-abiword";
  defaultExtMap["apng"] = "image/apng";
  defaultExtMap["arc"] = "application/x-freearc";
  defaultExtMap["avif"] = "image/avif";
  defaultExtMap["avi"] = "video/x-msvideo";
  defaultExtMap["azw"] = "application/vnd.amazon.ebook";
  defaultExtMap["bin"] = "application/octet-stream";
  defaultExtMap["bmp"] = "image/bmp";
  defaultExtMap["bz"] = "application/x-bzip";
  defaultExtMap["bz2"] = "application/x-bzip2";
  defaultExtMap["cda"] = "application/x-cdf";
  defaultExtMap["csh"] = "application/x-csh";
  defaultExtMap["css"] = "text/css";
  defaultExtMap["csv"] = "text/csv";
  defaultExtMap["doc"] = "application/msword";
  defaultExtMap["docx"] = "application/vnd.openxmlformats-officedocument.wordprocessingml.document";
  defaultExtMap["eot"] = "application/vnd.ms-fontobject";
  defaultExtMap["epub"] = "application/epub+zip";
  defaultExtMap["gif"] = "image/gif";
  defaultExtMap["gz"] = "application/gzip";
  defaultExtMap["htm"] = "text/html";
  defaultExtMap["html"] = "text/html";
  defaultExtMap["ico"] = "image/vnd.microsoft.icon";
  defaultExtMap["ics"] = "text/calendar";
  defaultExtMap["jar"] = "application/java-archive";
  defaultExtMap["jpg"] = "image/jpeg";
  defaultExtMap["jpeg"] = "image/jpeg";
  defaultExtMap["js"] = "text/javascript";
  defaultExtMap["json"] = "application/json";
  defaultExtMap["jsonld"] = "application/ld+json";
  defaultExtMap["mid"] = "audio/x-midi";
  defaultExtMap["midi"] = "audio/x-midi";
  defaultExtMap["mjs"] = "text/javascript";
  defaultExtMap["mp3"] = "audio/mp3";
  defaultExtMap["mp4"] = "video/mp4";
  defaultExtMap["mpeg"] = "video/mpeg";
  defaultExtMap["mpkg"] = "application/vnd.apple.installer+xml";
  defaultExtMap["mv4"] = "video/mp4";
  defaultExtMap["odp"] = "application/vnd.oasis.opendocument.presentation";
  defaultExtMap["ods"] = "application/vnd.oasis.opendocument.spreadsheet";
  defaultExtMap["odt"] = "application/vnd.oasis.opendocument.text";
  defaultExtMap["oga"] = "audio/ogg";
  defaultExtMap["ogg"] = "audio/ogg";
  defaultExtMap["ogv"] = "video/ogg";
  defaultExtMap["ogx"] = "application/ogg";
  defaultExtMap["opus"] = "audio/ogg";
  defaultExtMap["otf"] = "font/otf";
  defaultExtMap["png"] = "image/png";
  defaultExtMap["pdf"] = "application/pdf";
  defaultExtMap["php"] = "application/x-httpd-php";
  defaultExtMap["ppt"] = "application/vnd.ms-powerpoint";
  defaultExtMap["pptx"] = "application/vnd.openxmlformats-officedocument.presentationml.presentation";
  defaultExtMap["rar"] = "application/vnd.rar";
  defaultExtMap["rtf"] = "application/rtf";
  defaultExtMap["sh"] = "application/x-sh";
  defaultExtMap["svg"] = "image/svg+xml";
  defaultExtMap["swf"] = "application/x-shockwave-flash";
  defaultExtMap["tar"] = "application/x-tar";
  defaultExtMap["tif"] = "image/tiff";
  defaultExtMap["tiff"] = "image/tiff";
  defaultExtMap["ts"] = "video/mp2t";
  defaultExtMap["ttf"] = "font/ttf";
  defaultExtMap["txt"] = "text/plain";
  defaultExtMap["vsd"] = "application/vnd.visio";
  defaultExtMap["wasm"] = "application/wasm";
  defaultExtMap["wav"] = "audio/wav";
  defaultExtMap["weba"] = "audio/webm";
  defaultExtMap["webm"] = "video/webm";
  defaultExtMap["webp"] = "image/webp";
  defaultExtMap["woff"] = "font/woff";
  defaultExtMap["woff2"] = "font/woff2";
  defaultExtMap["xhtml"] = "application/xhtml+xml";
  defaultExtMap["xls"] = "application/vnd.ms-excel";
  defaultExtMap["xlsx"] = "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet";
  defaultExtMap["xml"] = "application/xml";
  defaultExtMap["xul"] = "application/vnd.mozilla.xul+xml";
  defaultExtMap["zip"] = "application/zip";
  return defaultExtMap;
}

ExtensionMap extMap = defaultExtensionMappings();

void readMapFromCsv(std::istream& f)
{
  int row = 0;
  while (f) {
    std::string line;
    getline(f, line);

    if (f) {
      row++;
      using CsvTokenizer = boost::tokenizer<boost::escaped_list_separator<char>>;
      CsvTokenizer tok(line);

      std::string extension, mime_type;
      int col = 0;
      for (auto i = tok.begin(); i != tok.end(); ++i, ++col) {
        switch (col)
        {
        case 0:
          extension = *i;
          boost::algorithm::to_lower(extension);
          break;

        case 1:
          mime_type = *i;
          boost::algorithm::to_lower(mime_type);
          break;

        default:
          break;
        }
      }
      if (col == 2) {
        extMap[extension] = mime_type;
      } else if (col) {
        LOG_WARN("Ignoring row " << row << " in file extension to MIME type mapping file due to incorrect amount of column.\n"
                 << "Correct formatting required two columns. First the extension, then the MIME type, in CSV format, thus separated by a comma.");
      }
    }
  }
}

void setMapping(const std::string& csvFile)
{
  extMap.clear();
  updateMapping(csvFile);
}

void updateMapping(const std::string& csvFile)
{
  std::ifstream f(csvFile.c_str());

  if (f) {
    readMapFromCsv(f);
  } else {
    extMap = defaultExtensionMappings();
    LOG_WARN("Could not find file with name " << csvFile << ". The default mapping for MIME types will be used.");
  }
}

std::string extensionToType(const std::string& extension)
{
  auto res = extMap.find(extension);
  if (res != extMap.end()) {
    return res->second;
  }
  return "application/octet-stream";
}

} // namespace mime_types
} // namespace server
} // namespace http
