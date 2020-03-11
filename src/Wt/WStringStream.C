/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <cstring>
#include <stdio.h>

#include "Wt/WStringStream.h"

#ifndef WT_DBO_STRINGSTREAM
#include <Wt/AsioWrapper/asio.hpp>

#include "WebUtils.h"
#endif // WT_DBO_STRINGSTREAM

#ifdef WT_WIN32
#define snprintf _snprintf
#endif

/*
 * Perhaps we should also implement reading from the stringstream,
 * this would be useful for Http::Client::Message::body()
 */

namespace Wt {

#ifdef WT_DBO_STRINGSTREAM
namespace Dbo {
#endif

WStringStream::WStringStream()
  : sink_(0),
    buf_(static_buf_),
    buf_i_(0)
{ }

WStringStream::WStringStream(std::ostream& sink)
  : sink_(&sink),
    buf_(static_buf_),
    buf_i_(0)
{ }

WStringStream& WStringStream::operator= (const WStringStream& other)
{
  clear();
  *this << other.str();
  return *this;
}

WStringStream::~WStringStream()
{
  flushSink();

  clear();
}

void WStringStream::clear()
{
  buf_i_ = 0;

  for (unsigned int i = 0; i < bufs_.size(); ++i)
    if (bufs_[i].first != static_buf_)
      delete[] bufs_[i].first;

  bufs_.clear();

  if (buf_ != static_buf_)
    delete[] buf_;

  buf_ = static_buf_;
}

bool WStringStream::empty() const
{
  return !sink_ && buf_i_ == 0 && bufs_.empty();
}

std::size_t WStringStream::length() const
{
  std::size_t result = buf_i_;

  for (unsigned int i = 0; i < bufs_.size(); ++i)
    result += bufs_[i].second;

  return result;
}

void WStringStream::flushSink()
{
  if (sink_) {
    sink_->write(buf_, buf_i_);
    buf_i_ = 0;
  }
}

void WStringStream::pushBuf()
{
  if (buf_i_ == 0)
    return;

  if (sink_) {
    sink_->write(buf_, buf_i_);
  } else {
    bufs_.push_back(std::make_pair(buf_, buf_i_));
    buf_ = new char[D_LEN];
  }

  buf_i_ = 0;
}

WStringStream& WStringStream::operator<< (char c)
{
  if (buf_i_ == buf_len())
    pushBuf();

  buf_[buf_i_++] = c;

  return *this;
}

WStringStream& WStringStream::operator<< (const std::string& s)
{
  append(s.data(), s.length());

  return *this;
}

WStringStream& WStringStream::operator<< (int v)
{
  char buf[20];
  Utils::itoa(v, buf);
  return *this << buf;
}

WStringStream& WStringStream::operator<< (unsigned int v)
{
  char buf[20];
  Utils::lltoa(v, buf);
  return *this << buf;
}

WStringStream& WStringStream::operator<< (bool v)
{
  if (v)
    return *this << "true";
  else
    return *this << "false";
}

WStringStream& WStringStream::operator<< (long long v)
{
  char buf[40];
  Utils::lltoa(v, buf);
  return *this << buf;
}

WStringStream& WStringStream::operator<< (double d)
{
  char buf[50];
  snprintf(buf, 50, "%g", d);
  return *this << buf;
}

void WStringStream::append(const char *s, int length)
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

const char *WStringStream::c_str()
{
  if (bufs_.empty()) {
    buf_[buf_i_] = 0;
    return buf_;
  } else
    return 0;
}

std::string WStringStream::str() const
{
  std::string result;
  result.reserve(length());

  for (unsigned int i = 0; i < bufs_.size(); ++i)
    result.append(bufs_[i].first, bufs_[i].second);

  result.append(buf_, buf_i_);

  return result;
}

#ifndef WT_DBO_STRINGSTREAM
void WStringStream::asioBuffers(std::vector<AsioWrapper::asio::const_buffer>& result) const
{
  result.reserve(result.size() + bufs_.size() + 1);

  for (unsigned int i = 0; i < bufs_.size(); ++i)
    result.push_back(AsioWrapper::asio::buffer(bufs_[i].first, bufs_[i].second));

  result.push_back(AsioWrapper::asio::buffer(buf_, buf_i_));
}
#endif

WStringStream::iterator WStringStream::back_inserter()
{
  return iterator(*this);
}

WStringStream::iterator::iterator()
  : stream_(0)
{ }

WStringStream::iterator::char_proxy WStringStream::iterator::operator * ()
{
  return char_proxy(*stream_);
}

WStringStream::iterator& WStringStream::iterator::operator ++ ()
{
  return *this;
}

WStringStream::iterator WStringStream::iterator::operator ++ (int)
{
  return *this;
}

WStringStream::iterator::char_proxy&
WStringStream::iterator::char_proxy::operator= (char c)
{
  stream_ << c;
  return *this;
}

WStringStream::iterator::char_proxy::char_proxy(WStringStream& stream)
  : stream_(stream)
{ }

WStringStream::iterator::iterator(WStringStream& stream)
  : stream_(&stream)
{ }

#ifdef WT_DBO_STRINGSTREAM
} // namespace Dbo
#endif

} // namespace Wt
