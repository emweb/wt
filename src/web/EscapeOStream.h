// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_ESCAPE_OSTREAM_H_
#define WT_ESCAPE_OSTREAM_H_

#include <Wt/WStringStream.h>

#ifndef WT_DBO_ESCAPEOSTREAM
#define WT_ESCAPEOSTREAM_API WT_API
#else // WT_DBO_ESCAPEOSTREAM
#define WT_ESCAPEOSTREAM_API WTDBO_API
#endif // WT_DBO_ESCAPEOSTREAM

namespace Wt {

#ifdef WT_DBO_ESCAPEOSTREAM
namespace Dbo {
#endif

class WT_ESCAPEOSTREAM_API EscapeOStream
{
public:
  enum RuleSet { Empty = 0, HtmlAttribute = 1,
		 JsStringLiteralSQuote = 2, JsStringLiteralDQuote = 3, 
                 Plain = 4, PlainTextNewLines = 5 };

  EscapeOStream();
  EscapeOStream(std::ostream& sink);
  EscapeOStream(WStringStream& sink);
  EscapeOStream(EscapeOStream& other);

  void pushEscape(RuleSet rules);
  void popEscape();

#ifdef WT_TARGET_JAVA
  EscapeOStream& push();
#endif // WT_TARGET_JAVA

  void append(const std::string& s, const EscapeOStream& rules);
  void append(const char *s, std::size_t len);

  EscapeOStream& operator<< (char);
  EscapeOStream& operator<< (const char *s)
  {
    if (c_special_ == 0)
      stream_ << s;
    else
      put(s, *this);

    return *this;
  }

  EscapeOStream& operator<< (const std::string& s);
  EscapeOStream& operator<< (int);
  EscapeOStream& operator<< (long long);
  EscapeOStream& operator<< (bool);
  EscapeOStream& operator<< (const EscapeOStream& other);

  const char *c_str(); // for default constructor, can return 0
  std::string str() const; // for default constructor

  bool empty() const;
  void clear();

private:
  WStringStream own_stream_;
  WStringStream& stream_;

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

  std::vector<RuleSet> ruleSets_;

  static const std::vector<Entry> standardSets_[6];
  static const std::string standardSetsSpecial_[6];

  static const Entry htmlAttributeEntries_[3];
  static const Entry jsStringLiteralSQuoteEntries_[5];
  static const Entry jsStringLiteralDQuoteEntries_[5];
  static const Entry plainTextEntries_[3];
  static const Entry plainTextNewLinesEntries_[4];
};

#ifdef WT_DBO_ESCAPEOSTREAM
} // namespace Dbo
#endif

} // namespace Wt

#endif // ESCAPE_OSTREAM_H_
