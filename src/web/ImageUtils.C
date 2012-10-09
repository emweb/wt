/*
 * Copyright (C) 2012 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "ImageUtils.h"
#include "FileUtils.h"
#include "WebUtils.h"

#include "Wt/WException"

#include <cstring>

namespace {
  const int mimeTypeCount = 10;
  const char *imageMimeTypes [] = { 
    "image/png", 
    "image/jpeg", 
    "image/gif", 
    "image/gif",
    "image/bmp",
    "image/bmp",
    "image/bmp",
    "image/bmp",
    "image/bmp",
    "image/bmp"
  };
  const char *imageHeaders [] = { 
    "\211PNG\r\n\032\n", 
    "\377\330\377",
    "GIF87a", 
    "GIF89a",
    "BA",
    "BM",
    "CI",
    "CP",
    "IC",
    "PI"
  };
  static const int imageHeaderSize [] = {
    8,
    3,
    6,
    6,
    2,
    2,
    2,
    2,
    2,
    2
  };
}

namespace Wt {
  namespace Image {
    std::string identifyImageFileMimeType(const std::string& fileName)
    {
      std::vector<unsigned char> header = FileUtils::fileHeader(fileName, 25);
      if (header.size() == 0)
	return "";
      return identifyImageMimeType(header);
    }
    
    std::string identifyImageMimeType(const std::vector<unsigned char>& header)
    {
      //TODO also check the filename extension, if parsing the file did not work

      for (int i = 0; i < mimeTypeCount; ++i) {
#ifndef WT_TARGET_JAVA
	if (std::memcmp(&header[0], 
			imageHeaders[i], imageHeaderSize[i]) == 0)
	  return std::string(imageMimeTypes[i]);
#else
	if (std::memcmp(header.data(), 
			imageHeaders[i], imageHeaderSize[i]) == 0)
	  return std::string(imageMimeTypes[i]);
#endif
      }

      return std::string();
    }
  }
}
