// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2012 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_FILE_UTILS_H_
#define WT_FILE_UTILS_H_

#include <string>
#include <vector>

#include <Wt/WDllDefs.h>

namespace Wt {
  namespace FileUtils {

    extern WT_API unsigned long long size(const std::string &file);
    extern WT_API time_t lastWriteTime(const std::string &file);
    extern WT_API bool exists(const std::string &file);
    extern bool isDirectory(const std::string &file);
    extern void listFiles(const std::string &directory, 
			  std::vector<std::string> &files);

    // Returns a filename that can be used as temporary file
    extern WT_API std::string createTempFileName();

  }
}

#endif // WT_FILE_UTILS_H_
