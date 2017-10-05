// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2012 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef IMAGE_UTILS_H_
#define IMAGE_UTILS_H_

#include <string>
#include <vector>

#include <Wt/WPoint.h>

namespace Wt {
  class ImageUtils {
  public:
    static std::string identifyMimeType(const std::vector<unsigned char>&
					header);

    static std::string identifyMimeType(const std::string& fileName);

    static WPoint getSize(const std::string& fileName);

    static WPoint getSize(const std::vector<unsigned char>& header);
    static WPoint getJpegSize(const std::string& fileName);

  };
}

#endif // IMAGE_UTILS_H_
