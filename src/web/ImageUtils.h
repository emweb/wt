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

#include <Wt/WDllDefs.h>

namespace Wt {
  namespace Image {

#ifdef WT_TARGET_JAVA	
    class ImageUtils {
    };
#endif //WT_TARGET_JAVA

    // Tries to identify the image mime type based on:
    // 1) the file content (header: first 25 bytes)
    // 2) the file name extension

    // If no mime type could be identified, an empty string is returned

    extern WT_API
    std::string identifyImageMimeType(const std::vector<unsigned char>& header);

    extern WT_API
    std::string identifyImageFileMimeType(const std::string& fileName);
  }
}

#endif // IMAGE_UTILS_H_
