// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2012 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_FILE_UTILS_H_
#define WT_FILE_UTILS_H_

#include <string>
#include <vector>
#include <ctime>

#include <Wt/WDllDefs.h>

namespace Wt {
  namespace FileUtils {

    extern WT_API 
    std::vector<unsigned char> fileHeader(const std::string &fileName, 
					  unsigned size);
    extern WT_API unsigned long long size(const std::string &file);
    extern WT_API std::string* fileToString(const std::string &fileName);
#ifndef WT_TARGET_JAVA
    extern WT_API std::time_t lastWriteTime(const std::string &file);
#endif
    extern WT_API bool exists(const std::string &file);
    extern bool isDirectory(const std::string &file);
    extern void listFiles(const std::string &directory, 
			  std::vector<std::string> &files);
    extern std::string leaf(const std::string &file);
    

    // Returns a filename that can be used as temporary file
    extern WT_API std::string createTempFileName();

    extern void appendFile(const std::string &srcFile,
			   const std::string &targetFile);
  }
}

#endif // WT_FILE_UTILS_H_
