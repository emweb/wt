/*
 * Copyright (C) 2012 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "ImageUtils.h"
#include "FileUtils.h"
#include "WebUtils.h"

#include "Wt/WException.h"
#include "Wt/WLogger.h"

#ifndef WT_TARGET_JAVA
#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>
#endif // WT_TARGET_JAVA

#include <boost/lexical_cast.hpp>

#include <cstring>

namespace {
  const int mimeTypeCount = 12;
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
    "image/bmp",
    "image/svg",
    "image/svg"
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
    "PI",
    "<?xml",
    "<svg"
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
    2,
    5,
    4
  };

#ifndef WT_TARGET_JAVA
#define toUnsigned unsigned
#else
  static unsigned toUnsigned(int c) {
    unsigned result = c;
    if (result < 0)
      result += 256;
    return result;
  }
#endif
}

namespace Wt {

LOGGER("ImageUtils");

std::string ImageUtils::identifyMimeType(const std::string& fileName)
{
  std::vector<unsigned char> header = FileUtils::fileHeader(fileName, 25);
 
  if (header.empty())
    return "";
  else
    return identifyMimeType(header);
}
    
std::string ImageUtils::identifyMimeType(const std::vector<unsigned char>&
					 header)
{
  // TODO: also check the filename extension, if parsing the file did not work
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

#ifndef WT_TARGET_JAVA
WPoint ImageUtils::getSize(const std::string& fileName)
{
  std::vector<unsigned char> header =
      FileUtils::fileHeader(fileName, 25);
 
  if (header.empty())
    return WPoint();
  else{
    std::string mimeType = identifyMimeType(header);
    if (mimeType == "image/jpeg")
      return getJpegSize(fileName);
    if (mimeType == "image/svg")
      return getSvgSize(fileName);
    else
      return getSize(header);
  }
}

WPoint ImageUtils::getSvgSize(const std::string& fileName)
{
  try {
    std::vector<unsigned char> headerv = FileUtils::fileHeader(fileName, 1024);
    std::string header(headerv.begin(), headerv.end());

    const char *wstr = std::strstr(header.c_str(), "width=\"");
    if (!wstr)
      return WPoint();
    const char *hstr = std::strstr(header.c_str(), "height=\"");
    if (!hstr)
      return WPoint();

    wstr += 7;
    hstr += 8;
    
    const char *wend = std::strstr(wstr, "\"");
    const char *hend = std::strstr(hstr, "\"");

    if (wend && hend) {
      double w = boost::lexical_cast<double>(std::string(wstr, wend));
      double h = boost::lexical_cast<double>(std::string(hstr, hend));
      return WPoint((int)w, (int)h);
    } else
      return WPoint();
  } catch (const boost::interprocess::interprocess_exception &e) {
    LOG_ERROR("getSvgSize: memory mapping SVG file '" <<
              fileName << "' failed with exception: " << e.what());
    return WPoint();
  }
}

WPoint ImageUtils::getJpegSize(const std::string& fileName)
{
  try {
    boost::interprocess::file_mapping mapping(fileName.c_str(), boost::interprocess::read_only);
    boost::interprocess::mapped_region region(mapping, boost::interprocess::read_only, 0, 2 * 1024 * 1024);
    const unsigned char *const header = static_cast<unsigned char*>(region.get_address());
    std::size_t headerSize = region.get_size();

    std::size_t pos = 2;

    if (pos + 12 > headerSize) {
      LOG_ERROR("getJpegSize: JPEG file '" <<
                fileName << "' is too small, size of mapped region: " <<
                headerSize << " bytes");
      return WPoint();
    }

    while (toUnsigned(header[pos])==0xFF) {
      if (toUnsigned(header[pos + 1])==0xC0 ||
          toUnsigned(header[pos + 1])==0xC1 ||
          toUnsigned(header[pos + 1])==0xC2 ||
          toUnsigned(header[pos + 1])==0xC3 ||
          toUnsigned(header[pos + 1])==0xC9 ||
          toUnsigned(header[pos + 1])==0xCA ||
          toUnsigned(header[pos + 1])==0xCB)
        break;
      pos += 2+ (toUnsigned(header[pos + 2])<<8) + toUnsigned(header[pos + 3]);
      if (pos + 12 > headerSize) {
        LOG_ERROR("getJpegSize: end of mapped region for JPEG file '" <<
                  fileName << "' reached without finding geometry, size of "
                  "mapped region: " << headerSize << " bytes");
        return WPoint();
      }
    }

    int height = (toUnsigned(header[pos + 5] << 8)) + toUnsigned(header[pos + 6]);
    int width = (toUnsigned(header[pos + 7] << 8)) + toUnsigned(header[pos + 8]);
    return WPoint(width, height);
  } catch (const boost::interprocess::interprocess_exception &e) {
    LOG_ERROR("getJpegSize: memory mapping JPEG file '" <<
              fileName << "' failed with exception: " << e.what());
    return WPoint();
  }
}

WPoint ImageUtils::getSize(const std::vector<unsigned char>& header)
{
  /*
   * Contributed by Daniel Derr @ ArrowHead Electronics Health-Care
   */
  std::string mimeType = identifyMimeType(header);

  if (mimeType == "image/png") {
    int width = ( ( ( toUnsigned(header[16]) << 8
		      | toUnsigned(header[17])) << 8
		    | toUnsigned(header[18])) << 8
		  | toUnsigned(header[19]));
    int height = ( ( ( toUnsigned(header[20]) << 8
		       | toUnsigned(header[21])) << 8
		     | toUnsigned(header[22])) << 8
		   | toUnsigned(header[23]));
    return WPoint(width, height);
  } else if (mimeType == "image/gif") {
    int width = toUnsigned(header[7]) << 8 | toUnsigned(header[6]);
    int height = toUnsigned(header[9]) << 8 | toUnsigned(header[8]);
    return WPoint(width, height);
  } else
    return WPoint();
}
#endif // WT_TARGET_JAVA

}
