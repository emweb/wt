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

namespace Wt {

class CgiParser;
class WebRequest;

class CgiEntry
{
public:
  std::string clientFilename() const { return cfn_; }
  std::string contentType() const { return ct_; }
  std::string value() const { return value_; }
  CgiEntry *next() const { return next_; }

  void stealFile();

  void setValue(const std::string value);

private:
  CgiEntry(const CgiEntry&);
  CgiEntry();
  ~CgiEntry();

  void setStream(const std::string spoolName,
		 const std::string clientFilename,
		 const std::string clientType);

  std::string cfn_;
  std::string ct_;
  std::string value_;
  bool isFile_;
  bool fileIsStolen_;

  CgiEntry *next_;

  friend class CgiParser;
};  

/*
 * Parses CGI in all its forms (get/post/file uploads).
 */
class CgiParser
{
public:
  typedef std::map<std::string, CgiEntry *> EntryMap;

  static void init();

  CgiParser(int maxPostData);
  ~CgiParser();

  /*
   * Reads in GET or POST data, converts it to unescaped text, and
   * creates Entry for each parameter entry. Multiple selections
   * are chained together.
   */
  void parse(WebRequest& request);

  CgiEntry *getEntry(const std::string key) const;
  const std::map<std::string, CgiEntry *>& entries()
    const { return entries_; }

  int postDataExceeded() const { return postDataExceeded_; }

private:
  unsigned maxPostData_;
  EntryMap entries_;

  void readMultipartData(WebRequest& request, const std::string type, int len);
  bool parseBody(WebRequest& request, const std::string boundary);
  bool parseHead(WebRequest& request);
  int left_;
  std::ostream *spoolStream_;
  CgiEntry *entry_;
  int postDataExceeded_;

  void readUntilBoundary(WebRequest& request, const std::string boundary,
			 int tossAtBoundary,
			 std::string *resultString,
			 std::ostream *resultFile);
  void windBuffer(int offset);
  int index(const std::string search);

  static const int BUFSIZE = 8192;
  static const int MAXBOUND = 100;

  int buflen_;
  static char buf_[BUFSIZE + MAXBOUND];

  CgiEntry *createEntry(std::string key);
  void replaceHexTokens(std::string& v);
  void replaceAll(std::string& v, char from, char to);
};

}

#endif // CGI_PARSER_H_
