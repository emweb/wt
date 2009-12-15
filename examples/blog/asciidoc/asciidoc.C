#include "asciidoc.h"

#include <fstream>
#include <signal.h>
#include <stdlib.h>
#include <string.h>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/convenience.hpp>

#include "Wt/WString"

using namespace Wt;
namespace fs = boost::filesystem;

namespace {

std::string tempFileName() 
{
#ifndef WIN32
  char spool[20];
  strcpy(spool, "/tmp/wtXXXXXX");

  int i = mkstemp(spool);
  close(i);
#else
  char spool[2 * L_tmpnam];
  tmpnam(spool);
#endif
  return std::string(spool);
}

std::string readFileToString(const std::string& fileName) 
{
  std::size_t outputFileSize = fs::file_size(fileName);
  std::fstream file (fileName.c_str(), std::ios::in | std::ios::binary);
  char* memblock = new char [outputFileSize];
  file.read(memblock, outputFileSize);
  file.close();
  std::string data = std::string(memblock, outputFileSize);
  delete [] memblock;
  return data;
}

}

WString asciidoc(const Wt::WString& src)
{
  std::string srcFileName = tempFileName();
  std::string htmlFileName = srcFileName + ".html";

  {
    std::ofstream srcFile(srcFileName.c_str(), std::ios::out);
    std::string ssrc = src.toUTF8();
    srcFile.write(ssrc.c_str(), ssrc.length());
    srcFile.close();
  }

  std::string command = "asciidoc -s " + srcFileName;

#ifndef WIN32
  /*
   * So, asciidoc apparently sends a SIGINT which is caught by its parent
   * process.. So we have to temporarily ignore it.
   */
  struct sigaction newAction, oldAction;
  newAction.sa_handler = SIG_IGN;
  newAction.sa_flags = 0;
  sigemptyset(&newAction.sa_mask);
  sigaction(SIGINT, &newAction, &oldAction);
#endif
  bool ok = system(command.c_str()) == 0;
#ifndef WIN32
  sigaction(SIGINT, &oldAction, 0);
#endif

  WString result;

  if (ok) {
    result = WString::fromUTF8(readFileToString(htmlFileName));
  } else
    result = WString::fromUTF8("<i>Could not execute asciidoc</i>");

  unlink(srcFileName.c_str());
  unlink(htmlFileName.c_str());

  return result;
}
