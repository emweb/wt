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

SStream::SStream()
  : sink_(0),
    buf_(static_buf_),
    buf_i_(0)
{ }

SStream::SStream(std::ostream& sink)
  : sink_(&sink),
    buf_(static_buf_),
    buf_i_(0)
{ }

SStream::~SStream()
{
  flushSink();

  for (unsigned int i = 1; i < bufs_.size(); ++i)
    delete[] bufs_[i].first;

  if (buf_ != static_buf_)
    delete[] buf_;
}

void SStream::clear()
{
  buf_i_ = 0;
}

bool SStream::empty() const
{
  return !sink_ && buf_ == static_buf_ && buf_i_ == 0;
}

void SStream::flushSink()
{
  if (sink_) {
    sink_->write(buf_, buf_i_);
    buf_i_ = 0;
  }
}

void SStream::pushBuf()
{
  if (sink_) {
    sink_->write(buf_, buf_i_);
    buf_i_ = 0;
  } else {
    bufs_.push_back(std::make_pair(buf_, buf_i_));
    buf_ = new char[D_LEN];
    buf_i_ = 0;
  }
}

SStream& SStream::operator<< (char c)
{
  if (buf_i_ == buf_len())
    pushBuf();

  buf_[buf_i_++] = c;

  return *this;
}

SStream& SStream::operator<< (const char *s)
{
  append(s, std::strlen(s));

  return *this;
}

SStream& SStream::operator<< (const std::string& s)
{
  append(s.data(), s.length());

  return *this;
}

SStream& SStream::operator<< (int v)
{
  char buf[20];
  Utils::itoa(v, buf);
  return *this << buf;
}

void SStream::append(const char *s, int length)
{
  if (buf_i_ + length > buf_len()) {
    pushBuf();

    if (length > buf_len()) {
      if (sink_) {
	sink_->write(s, length);
	return;
      } else {
	char *buf = new char[length];
	std::memcpy(buf, s, length);
	bufs_.push_back(std::make_pair(buf, length));
	return;
      }
    }
  }

  std::memcpy(buf_ + buf_i_, s, length);
  buf_i_ += length;
}

const char *SStream::c_str()
{
  if (bufs_.empty()) {
    buf_[buf_i_] = 0;
    return buf_;
  } else
    return 0;
}

std::string SStream::str() const
{
  int length = buf_i_;
  for (unsigned int i = 0; i < bufs_.size(); ++i)
    length += bufs_[i].second;

  std::string result;
  result.reserve(length);

  for (unsigned int i = 0; i < bufs_.size(); ++i)
    result.append(bufs_[i].first, bufs_[i].second);

  result.append(buf_, buf_i_);

  return result;
}

SStream::iterator SStream::back_inserter()
{
  return iterator(*this);
}

SStream::iterator::iterator()
  : stream_(0)
{ }

SStream::iterator::char_proxy SStream::iterator::operator * ()
{
  return char_proxy(*stream_);
}

SStream::iterator& SStream::iterator::operator ++ ()
{
  return *this;
}

SStream::iterator SStream::iterator::operator ++ (int)
{
  return *this;
}

SStream::iterator::char_proxy&
SStream::iterator::char_proxy::operator= (char c)
{
  stream_ << c;
  return *this;
}

SStream::iterator::char_proxy::char_proxy(SStream& stream)
  : stream_(stream)
{ }

SStream::iterator::iterator(SStream& stream)
  : stream_(&stream)
{ }

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
  : c_special_(0)
{ }

EscapeOStream::EscapeOStream(std::ostream& sink)
  : stream_(sink),
    c_special_(0)
{ }

EscapeOStream::EscapeOStream(EscapeOStream& other)
  : mixed_(other.mixed_),
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

EscapeOStream& EscapeOStream::operator<< (const char *s)
{
  if (c_special_ == 0)
    stream_ << s;
  else
    put(s, *this);

  return *this;
}

void EscapeOStream::append(const std::string& s, const EscapeOStream& rules)
{
  if (rules.c_special_ == 0)
    stream_ << s;
  else
    put(s.data(), rules);
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
      stream_ << s;
      s = 0;
    }
  }
}

EscapeOStream& EscapeOStream::operator<< (int arg)
{
  stream_ << arg;

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


}
