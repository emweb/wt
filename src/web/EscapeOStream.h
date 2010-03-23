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

/*
 * A faster stringstream than the standard library, probably because
 * of no overhead w.r.t. localization
 */
class WT_API SStream {
public:
  struct iterator {
    struct char_proxy {
      char_proxy& operator= (char c);

    private:
      char_proxy(SStream& stream);
      SStream& stream_;

      friend struct iterator;
    };

    iterator();

    char_proxy operator * ();

    iterator& operator ++ ();
    iterator  operator ++ (int);

  private:
    SStream *stream_;
    iterator(SStream& stream);

    friend class SStream;
  };

  SStream();
  SStream(std::ostream& sink);
  ~SStream();

  void append(const char *s, int length);

  SStream& operator<< (char);
  SStream& operator<< (const char *s);
  SStream& operator<< (const std::string& s);
  SStream& operator<< (int);

  iterator back_inserter();

  const char *c_str();
  std::string str() const;

  bool empty() const;
  void clear();

private:
  enum {S_LEN = 1024};
  enum {D_LEN = 2048};

  std::ostream *sink_;
  char static_buf_[S_LEN + 1];

  char *buf_;
  int buf_i_;

  int buf_len() const { return buf_ == static_buf_
      ? static_cast<int>(S_LEN) : static_cast<int>(D_LEN); }

  std::vector<std::pair<char *, int> > bufs_;

  void flushSink();
  void pushBuf();
};

class WT_API EscapeOStream
{
public:
  enum RuleSet { Empty = 0, HtmlAttribute = 1,
		 JsStringLiteralSQuote = 2, JsStringLiteralDQuote = 3, 
                 PlainText = 4, PlainTextNewLines = 5 };

  EscapeOStream();
  EscapeOStream(std::ostream& sink);
  EscapeOStream(EscapeOStream& other);

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
  EscapeOStream& operator<< (const EscapeOStream& other);

  const char *c_str(); // for default constructor, can return 0
  std::string str() const; // for default constructor

  bool empty() const;
  void clear();

private:
  SStream stream_;

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

}

#endif // ESCAPE_OSTREAM_H_
