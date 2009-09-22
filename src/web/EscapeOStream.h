// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef ESCAPE_OSTREAM_H_
#define ESCAPE_OSTREAM_H_

#include <iostream>
#include <string>
#include <vector>
#include <Wt/WDllDefs.h>

namespace Wt {

class WT_API EscapeOStream
{
public:
  enum RuleSet { Empty = 0, HtmlAttribute = 1,
		 JsStringLiteralSQuote = 2, JsStringLiteralDQuote = 3 };

  EscapeOStream(); // only for strings short than 1024 chars,
                   // get using c_str() !
  EscapeOStream(std::ostream& sink);
  EscapeOStream(EscapeOStream& other);
  ~EscapeOStream();

  void pushEscape(RuleSet rules);
  void popEscape();

#ifdef WT_TARGET_JAVA
  EscapeOStream& push();
#endif // WT_TARGET_JAVA

  void append(const std::string& s, const EscapeOStream& rules);

  EscapeOStream& operator<< (char);
  EscapeOStream& operator<< (const char *s);
  EscapeOStream& operator<< (const std::string& s);
  EscapeOStream& operator<< (int);

  const char *c_str();
  void clear();

private:
  static const int S_LEN = 1024;

  std::ostream *sink_;
  char s_[S_LEN];
  int  slen_;

  struct Entry {
    char c;
    std::string s;
  };
  std::vector<Entry> mixed_;
  std::string special_;
  const char *c_special_;

  void mixRules();
  void put(const char *s, const EscapeOStream& rules);

  void sAppend(char c);
  void sAppend(const char *s, int length);
  void sAppend(const std::string& s);
  void flush();

  std::vector<RuleSet> ruleSets_;

  static const std::vector<Entry> standardSets_[4];
  static const std::string standardSetsSpecial_[4];

  static const Entry htmlAttributeEntries_[3];
  static const Entry jsStringLiteralSQuoteEntries_[5];
  static const Entry jsStringLiteralDQuoteEntries_[5];
};

}

#endif // ESCAPE_OSTREAM_H_
