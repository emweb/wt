/*
 * Copyright (C) 2012 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "web/FileUtils.h"

#include <boost/filesystem/operations.hpp>

#include "web/WebUtils.h"
#include "Wt/WException.h"
#include "Wt/WLogger.h"

#ifdef WT_WIN32
#include <windows.h>
#endif // WIN32

#include <fstream>

namespace Wt {
  LOGGER("FileUtils");

  namespace FileUtils {
    std::vector<unsigned char> fileHeader(const std::string &fileName,
                                          unsigned size)
    {
      std::vector<unsigned char> header;

      std::ifstream file;
      file.open(fileName.c_str(), std::ios::binary | std::ios::in);

      if (file.good()) {
        file.seekg(0, std::ios::beg);

        header.resize(size);
        file.read((char*)&header[0], size);
        file.close();

        return header;
      } else {
        return header;
      }
    }

    unsigned long long size(const std::string &file)
    {
      return (unsigned long long) boost::filesystem::file_size(file);
    }

    std::string* fileToString(const std::string& fileName)
    {
      std::ifstream ifs(fileName.c_str());
      if(!ifs)
        return 0;
      else
        return new std::string((std::istreambuf_iterator<char>(ifs)),
                     std::istreambuf_iterator<char>());
    }

    std::chrono::system_clock::time_point lastWriteTime(const std::string &file)
    {
      return std::chrono::system_clock::from_time_t(boost::filesystem::last_write_time(file));
    }

    bool exists(const std::string &file)
    {
      boost::filesystem::path path(file);
      return boost::filesystem::exists(path);
    }

    bool isDirectory(const std::string &file)
    {
      boost::filesystem::path path(file);
      return boost::filesystem::is_directory(path);
    }

    void listFiles(const std::string &directory,
                   std::vector<std::string> &files)
    {
      boost::filesystem::path path(directory);
      boost::filesystem::directory_iterator end_itr;

      if (!boost::filesystem::is_directory(path)) {
        std::string error
          = "listFiles: \"" + directory + "\" is not a directory";
        LOG_ERROR(error);
        throw WException(error);
      }

      for (boost::filesystem::directory_iterator i(path); i != end_itr; ++i) {
        std::string f = (*i).path().string();
        files.push_back(f);
      }
    }

    std::string getTempDir()
    {
      std::string tempDir;

      char *wtTmpDir = std::getenv("WT_TMP_DIR");
      if (wtTmpDir)
      tempDir = wtTmpDir;
      else {
#ifdef WT_WIN32
        char winTmpDir[MAX_PATH];
        if(GetTempPathA(sizeof(winTmpDir), winTmpDir) != 0)
          tempDir = winTmpDir;
#else
        tempDir = "/tmp";
#endif
      }

      return tempDir;
    }

    extern std::string createTempFileName()
    {
      std::string tempDir = getTempDir();

#ifdef WT_WIN32
      char tmpName[MAX_PATH];

      if(tempDir == ""
         || GetTempFileNameA(tempDir.c_str(), "wt-", 0, tmpName) == 0)
        return "";

      return tmpName;
#else
      char* spool = new char[20 + tempDir.size()];
      strcpy(spool, (tempDir + "/wtXXXXXX").c_str());

      int i = mkstemp(spool);
      close(i);

      std::string returnSpool = spool;
      delete [] spool;
      return returnSpool;
#endif
    }

    std::string leaf(const std::string &file)
    {
      #ifdef WT_WIN32
      char separator = '\\';
      #else
      char separator = '/';
      #endif

      std::size_t pos = file.rfind(separator);
      if (pos != std::string::npos)
        return file.substr(pos + 1);
      else
        return file;
    }

    void appendFile(const std::string &srcFile,
                    const std::string &targetFile)
    {
      std::ifstream ss(srcFile.c_str(),
                       std::ios::in | std::ios::binary);
      std::ofstream ts(targetFile.c_str(),
                       std::ios::out | std::ios::binary | std::ios::app);

      const int LEN = 4096;
      char buffer[LEN];
      while (!ss.eof()) {
        ss.read(buffer, LEN);
        ts.write(buffer, ss.gcount());
      }
    }

  }
}
