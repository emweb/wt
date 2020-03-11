/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "EscapeOStream.h"

#ifndef WT_DBO_ESCAPEOSTREAM
#include "WebUtils.h"
#endif

namespace Wt {

#ifdef WT_DBO_ESCAPEOSTREAM
namespace Dbo {
#endif

const EscapeOStream::Entry EscapeOStream::htmlAttributeEntries_[] = {
  { '&', "&amp;" },
  { '\"', "&#34;" },
  { '<', "&lt;" }
};

const EscapeOStream::Entry EscapeOStream::plainTextEntries_[] = {
  { '&', "&amp;" },
  { '>', "&gt;" },
  { '<', "&lt;" }
};

const EscapeOStream::Entry EscapeOStream::plainTextNewLinesEntries_[] = {
  { '&', "&amp;" },
  { '>', "&gt;" },
  { '<', "&lt;" },
  { '\n', "<br />" }
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
  std::vector<EscapeOStream::Entry>(plainTextEntries_,
				    plainTextEntries_ + 3),
  std::vector<EscapeOStream::Entry>(plainTextNewLinesEntries_,
				    plainTextNewLinesEntries_ + 4)
};

const std::string EscapeOStream::standardSetsSpecial_[] = { 
  std::string(),
  std::string("&\"<"),
  std::string("\\\n\r\t'"),
  std::string("\\\n\r\t\""),
  std::string("&><"),
  std::string("&><\n")
};

EscapeOStream::EscapeOStream()
  : stream_(own_stream_),
    c_special_(0)
{ }

EscapeOStream::EscapeOStream(std::ostream& sink)
  : own_stream_(sink),
    stream_(own_stream_),
    c_special_(0)
{ }

EscapeOStream::EscapeOStream(WStringStream& sink)
  : stream_(sink),
    c_special_(0)
{ }

EscapeOStream::EscapeOStream(EscapeOStream& other)
  : stream_(own_stream_),
    mixed_(other.mixed_),
    special_(other.special_),
    c_special_(special_.empty() ? 0 : special_.c_str()),
    ruleSets_(other.ruleSets_)
{ }

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
	    Utils::replace(mixed_[j].s, toMix[k].c, toMix[k].s);

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
    stream_ << c;
  } else {
    std::size_t i = special_.find(c);

    if (i != std::string::npos)
      stream_ << mixed_[i].s;
    else
      stream_ << c;
  }

  return *this;
}

void EscapeOStream::append(const char *s, std::size_t len)
{
  if (c_special_ == 0)
    stream_.append(s, len);
  else
    put(s, *this);
}

void EscapeOStream::append(const std::string& s, const EscapeOStream& rules)
{
  if (rules.c_special_ == 0)
    stream_ << s;
  else
    put(s.c_str(), rules);
}

EscapeOStream& EscapeOStream::operator<< (const std::string& s)
{
  append(s, *this);

  return *this;
}

EscapeOStream& EscapeOStream::operator<< (int i)
{
  stream_ << i;

  return *this;
}

EscapeOStream& EscapeOStream::operator<< (long long i)
{
  stream_ << i;

  return *this;
}

void EscapeOStream::put(const char *s, const EscapeOStream& rules)
{
  for (;s;) {
    const char *f = std::strpbrk(s, rules.c_special_);
    if (f != 0) {
      stream_.append(s, static_cast<int>(f - s));
      
      unsigned i = 0;
      for (; i < rules.mixed_.size(); ++i)
	if (rules.mixed_[i].c == *f) {
	  stream_ << rules.mixed_[i].s;
	  break;
	}

      if (i == rules.mixed_.size())
	stream_ << *f;

      s = f + 1;
    } else {
      stream_ << const_cast<char *>(s);
      s = 0;
    }
  }
}

EscapeOStream& EscapeOStream::operator<< (bool b)
{
  stream_ << b;

  return *this;
}

EscapeOStream& EscapeOStream::operator<< (const EscapeOStream& other)
{
  if (!other.empty())
    *this << other.str(); // FIXME could be optimized ?

  return *this;
}

const char *EscapeOStream::c_str()
{
  return stream_.c_str();
}

std::string EscapeOStream::str() const
{
  return stream_.str();
}

void EscapeOStream::clear()
{
  stream_.clear();
}

bool EscapeOStream::empty() const
{
  return stream_.empty();
}

#ifdef WT_DBO_ESCAPEOSTREAM
} // namespace Dbo
#endif

} // namespace Wt
