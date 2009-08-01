/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <cstring>
#include <boost/lexical_cast.hpp>
#include "EscapeOStream.h"
#include "Utils.h"
#include "WtException.h"

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

EscapeOStream::EscapeOStream()
  : sink_(0),
    slen_(0),
    c_special_(0)
{ }

EscapeOStream::EscapeOStream(std::ostream& sink)
  : sink_(&sink),
    slen_(0),
    c_special_(0)
{ }

EscapeOStream::EscapeOStream(EscapeOStream& other)
  : sink_(0),
    slen_(0),
    mixed_(other.mixed_),
    special_(other.special_),
    c_special_(special_.empty() ? 0 : special_.c_str()),
    ruleSets_(other.ruleSets_)
{ }

EscapeOStream::~EscapeOStream()
{
  if (sink_)
    flush();
}

void EscapeOStream::flush()
{
  sink_->write(s_, slen_);
  slen_ = 0;
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
  if (c_special_ == 0) {
    sAppend(c);
  } else {
    std::size_t i = special_.find(c);

    if (i != std::string::npos)
      sAppend(mixed_[i].s);
    else
      sAppend(c);
  }

  return *this;
}

EscapeOStream& EscapeOStream::operator<< (const char *s)
{
  if (c_special_ == 0)
    sAppend(s, std::strlen(s));
  else
    put(s, *this);

  return *this;
}

void EscapeOStream::append(const std::string& s, const EscapeOStream& rules)
{
  if (rules.c_special_ == 0)
    sAppend(s);
  else
    put(s.c_str(), rules);
}

EscapeOStream& EscapeOStream::operator<< (const std::string& s)
{
  append(s, *this);

  return *this;
}

void EscapeOStream::put(const char *s, const EscapeOStream& rules)
{
  for (;s;) {
    const char *f = std::strpbrk(s, rules.c_special_);
    if (f != 0) {
      sAppend(s, (f - s));
      
      unsigned i = 0;
      for (; i < rules.mixed_.size(); ++i)
	if (rules.mixed_[i].c == *f) {
	  sAppend(rules.mixed_[i].s);
	  break;
	}

      if (i == rules.mixed_.size())
	sAppend(*f);

      s = f + 1;
    } else {
      sAppend(s, std::strlen(s));
      s = 0;
    }
  }
}

EscapeOStream& EscapeOStream::operator<< (int arg)
{
  sAppend(boost::lexical_cast<std::string>(arg));

  return *this;
}

void EscapeOStream::sAppend(char c)
{
  if (slen_ == S_LEN) {
    if (!sink_)
      throw WtException("EscapeOStream buffer too short");
    sink_->write(s_, slen_);
    slen_ = 0;
  }

  s_[slen_++] = c;
}

void EscapeOStream::sAppend(const char *s, int length)
{
  if (slen_ + length > S_LEN) {
    if (!sink_)
      throw WtException("EscapeOStream buffer too short");
    sink_->write(s_, slen_);
    slen_ = 0;

    if (length > S_LEN) {
      sink_->write(s, length);
      return;
    }
  }

  std::memcpy(s_ + slen_, s, length);
  slen_ += length;
}

const char *EscapeOStream::c_str()
{
  s_[slen_] = 0;
  return s_;
}

void EscapeOStream::clear()
{
  slen_ = 0;
}

void EscapeOStream::sAppend(const std::string& s)
{
  sAppend(s.c_str(), s.length());
}

}
