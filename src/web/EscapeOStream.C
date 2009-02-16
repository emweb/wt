/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <cstring>
#include <boost/lexical_cast.hpp>
#include "EscapeOStream.h"
#include "Utils.h"

namespace Wt {

const EscapeOStream::Entry EscapeOStream::htmlAttributeEntries_[] = {
  { '&', "&amp;" },
  { '\"', "&#34;" },
  { '<', "&lt;" }
};

const EscapeOStream::Entry EscapeOStream::jsStringLiteralSQuoteEntries_[] = {
  { '\\', "\\\\" },
  { '\n', "\\n" },
  { '\r', "\\r" },
  { '\t', "\\t" },
  { '\'', "\\'" },
};

const EscapeOStream::Entry EscapeOStream::jsStringLiteralDQuoteEntries_[] = {
  { '\\', "\\\\" },
  { '\n', "\\n" },
  { '\r', "\\r" },
  { '\t', "\\t" },
  { '"', "\\\"" },
};

const std::vector<EscapeOStream::Entry> EscapeOStream::standardSets_[] = { 
  std::vector<EscapeOStream::Entry>(),
  std::vector<EscapeOStream::Entry>(htmlAttributeEntries_,
				    htmlAttributeEntries_ + 3),
  std::vector<EscapeOStream::Entry>(jsStringLiteralSQuoteEntries_,
				    jsStringLiteralSQuoteEntries_ + 5),
  std::vector<EscapeOStream::Entry>(jsStringLiteralDQuoteEntries_,
				    jsStringLiteralDQuoteEntries_ + 5),
};

const std::string EscapeOStream::standardSetsSpecial_[] = { 
  std::string(),
  std::string("&\"<"),
  std::string("\\\n\r\t'"),
  std::string("\\\n\r\t\"")
};


EscapeOStream::EscapeOStream(std::ostream& sink)
  : sink_(sink),
    c_special_(0)
{ }

EscapeOStream::EscapeOStream(EscapeOStream& other)
  : sink_(other.sink_),
    mixed_(other.mixed_),
    special_(other.special_),
    c_special_(special_.empty() ? 0 : special_.c_str())
{ }

EscapeOStream::~EscapeOStream()
{
  flush();
}

void EscapeOStream::flush()
{
  sink_ << s_;
  s_.clear();
}

void EscapeOStream::mixRules()
{
  mixed_.clear();
  special_.clear();

  const int ruleSetsSize = ruleSets_.size();

  if (ruleSetsSize == 0) {
    c_special_ = 0;
  } else { 
    if (ruleSetsSize == 1) {
      mixed_ = standardSets_[ruleSets_[0]];
      special_ = standardSetsSpecial_[ruleSets_[0]];
    } else
      for (int i = ruleSetsSize - 1; i >= 0; --i) {
	const std::vector<Entry>& toMix = standardSets_[ruleSets_[i]];

	for (unsigned j = 0; j < mixed_.size(); ++j)
	  for (unsigned k = 0; k < toMix.size(); ++k)
	    Wt::Utils::replace(mixed_[j].s, toMix[k].c, toMix[k].s);

	mixed_.insert(mixed_.end(), toMix.begin(), toMix.end());

	for (unsigned j = 0; j < toMix.size(); ++j)
	  special_.push_back(toMix[j].c);
      }

    if (!special_.empty())
      c_special_ = special_.c_str();
    else
      c_special_ = 0;
  }
}

void EscapeOStream::pushEscape(RuleSet rules)
{
  ruleSets_.push_back(rules);
  mixRules();
}

void EscapeOStream::popEscape()
{
  ruleSets_.pop_back();
  mixRules();
}

EscapeOStream& EscapeOStream::operator<< (char c)
{
  if (c_special_ == 0)
    s_.push_back(c);
  else {
    std::size_t i = special_.find(c);

    if (i != std::string::npos)
      s_ += mixed_[i].s;
    else
      s_.push_back(c);
  }

  return *this;
}

EscapeOStream& EscapeOStream::operator<< (const char *s)
{
  if (c_special_ == 0) {
    s_.append(s);
    return *this;
  }

  unsigned l = std::strlen(s);
  s_.reserve(s_.length() + 2*l);
  put(s);

  return *this;
}

EscapeOStream& EscapeOStream::operator<< (const std::string& s)
{
  if (c_special_ == 0) {
    s_.append(s);
    return *this;
  }

  s_.reserve(s_.length() + 2*s.length());
  put(s.c_str());

  return *this;
}

void EscapeOStream::put(const char *s)
{
  for (;s;) {
    const char *f = std::strpbrk(s, c_special_);
    if (f != 0) {
      s_.append(s, (f - s));
      
      unsigned i = 0;
      for (; i < mixed_.size(); ++i)
	if (mixed_[i].c == *f) {
	  s_ += mixed_[i].s;
	  break;
	}

      if (i == mixed_.size())
	s_.push_back(*f);

      s = f + 1;
    } else {
      s_.append(s);
      s = 0;
    }
  }
}

EscapeOStream& EscapeOStream::operator<< (int arg)
{
  s_.append(boost::lexical_cast<std::string>(arg));

  return *this;
}

}
