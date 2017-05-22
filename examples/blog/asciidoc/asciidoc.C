#include "asciidoc.h"

#include <fstream>
#include <signal.h>
#include <stdlib.h>
#include <string.h>

#include "Wt/WString.h"

#ifndef WT_WIN32
#include <unistd.h>
#endif

namespace {

std::string tempFileName() 
{
#ifndef WT_WIN32
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
  std::fstream file(fileName.c_str(), std::ios::in | std::ios::binary | std::ios::ate);
  int length = file.tellg();
  file.seekg(0, std::ios::beg);

  std::unique_ptr<char[]> buf(new char[length]);
  file.read(buf.get(), length);
  file.close();

  return std::string(buf.get(), length);
}

}

Wt::WString asciidoc(const Wt::WString& src)
{
  std::string srcFileName = tempFileName();
  std::string htmlFileName = tempFileName();

  {
    std::ofstream srcFile(srcFileName.c_str(), std::ios::out);
    std::string ssrc = src.toUTF8();
    srcFile.write(ssrc.c_str(), (std::streamsize)ssrc.length());
    srcFile.close();
  }

#if defined(ASCIIDOC_EXECUTABLE)
#define xstr(s) str(s)
#define str(s) #s
  std::string cmd = xstr(ASCIIDOC_EXECUTABLE);
#else
  std::string cmd = "asciidoc";
#endif
  std::string command = cmd + " -o " + htmlFileName + " -s " + srcFileName;

#ifndef WT_WIN32
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
#ifndef WT_WIN32
  sigaction(SIGINT, &oldAction, 0);
#endif

  Wt::WString result;

  if (ok) {
    result = Wt::WString(readFileToString(htmlFileName));
  } else
    result = Wt::WString("<i>Could not execute asciidoc</i>");

  unlink(srcFileName.c_str());
  unlink(htmlFileName.c_str());

  return result;
}
