// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef CGI_PARSER_H_
#define CGI_PARSER_H_

#include <string>
#include <map>
#include <iostream>
#include <vector>
#include <boost/cstdint.hpp>

#include <Wt/WDllDefs.h>

namespace Wt {

class CgiParser;
class WebRequest;

/*
 * Parses CGI in all its forms (get/post/file uploads).
 */
class CgiParser
{
public:
  enum ReadOption { ReadDefault, ReadHeadersOnly, ReadBodyAnyway };

  static void init();

  CgiParser(::int64_t maxPostData);

  /*
   * Reads in GET or POST data, converts it to unescaped text, and
   * creates Entry for each parameter entry. The request is annotated
   * with the parse results.
   */
  void parse(WebRequest& request, ReadOption option);

private:
  void readMultipartData(WebRequest& request, const std::string type,
			 ::int64_t len);
  bool parseBody(WebRequest& request, const std::string boundary);
  bool parseHead(WebRequest& request);
  ::int64_t maxPostData_, left_;
  std::ostream *spoolStream_;
  WebRequest *request_;

  std::string currentKey_;

  void readUntilBoundary(WebRequest& request, const std::string boundary,
			 int tossAtBoundary,
			 std::string *resultString,
			 std::ostream *resultFile);
  void windBuffer(int offset);
  int index(const std::string search);

  enum {BUFSIZE = 8192};
  enum {MAXBOUND = 100};

  int buflen_;
  char buf_[BUFSIZE + MAXBOUND];
};

}

#endif // CGI_PARSER_H_
