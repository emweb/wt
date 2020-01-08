/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <Wt/WException.h>
#include "FCGIRecord.h"

#include <unistd.h>
#include <errno.h>
#include <cstdlib>
#include <cstring>

using std::malloc;
using std::free;
using std::realloc;
using std::memcpy;

FCGIRecord::FCGIRecord()
  : good_(false),
    contentData_(nullptr),
    plainTextBuf_(nullptr)
{ }

FCGIRecord::FCGIRecord(short requestId, char version)
  : FCGIRecord()
{
  plainTextBufLength_ = 8 + 8;
  plainTextBuf_ = (unsigned char *) malloc(plainTextBufLength_ * sizeof(char));
  plainTextBuf_[0] = version;
  plainTextBuf_[1] = 3;  // FCGI_END_REQUEST
  plainTextBuf_[2] = requestId >> 8;
  plainTextBuf_[3] = requestId & 0x0F;
  plainTextBuf_[4] = 0;
  plainTextBuf_[5] = 8;
  plainTextBuf_[6] = 0;
  plainTextBuf_[7] = 0;
  
  plainTextBuf_[8] = 0;
  plainTextBuf_[9] = 0;
  plainTextBuf_[10] = 0;
  plainTextBuf_[11] = 0;
  plainTextBuf_[12] = 0; // FCGI_REQUEST_COMPLETE
}

FCGIRecord::~FCGIRecord()
{
  clear();
}

void FCGIRecord::clear()
{
  delete [] contentData_;

  if (plainTextBuf_)
    free(plainTextBuf_);

  contentData_ = nullptr;
  plainTextBufLength_ = 0;
  plainTextBuf_ = nullptr;
  plainTextLength_ = 0;
}

int FCGIRecord::getChar(int fd, bool waitForIt)
{
  unsigned char buf[1];

  int result;
  for (;;) {
    result = ::read(fd, buf, 1);
    if (result == -1) {
      if (errno != EINTR) {
	perror("read");
	throw Wt::WException("Error reading (1)");
      }
    } else
      break;
  }

  if (result == 0) {
    if (waitForIt) {
      while (result == 0) {
	usleep(100);
	result = ::read(fd, buf, 1);
	if (result == -1) {
	  if (errno != EINTR) {
	    perror("read");
	    throw Wt::WException("Error reading (2)");
	  } else
	    result = 0; // try again
	}
      }
    } else
      return -1;
  }
     
  if (plainTextLength_ >= plainTextBufLength_) {
    plainTextBufLength_ += 1024;
    plainTextBuf_ = (unsigned char *)
      realloc(plainTextBuf_, plainTextBufLength_);
  }

  plainTextBuf_[plainTextLength_++] = buf[0];

  return buf[0];
}

bool FCGIRecord::getAndAssign(int fd, unsigned char& result,
			      bool waitForIt)
{
  int c = getChar(fd, waitForIt);
  if (c != -1) {
    result = c;
    return true;
  } else {
    return false;
  }
}

bool FCGIRecord::getBuffer(int fd, unsigned char *buf,
			   int length)
{
  int count = 0;

  while (count < length) {
    int result = ::read(fd, buf + count, length - count);
    if (result == -1) {
      if (errno != EINTR) {
	perror("read");
	throw Wt::WException("Error reading (3)");
      } // else try again
    } else {
      count += result;
    }
  }

  if (plainTextLength_ + length > plainTextBufLength_) {
    plainTextBufLength_ += length;
    plainTextBufLength_ = (plainTextBufLength_ + 8) / 8 * 8;
    plainTextBuf_ = (unsigned char *)
      realloc(plainTextBuf_, plainTextBufLength_);
  }

  memcpy(plainTextBuf_ + plainTextLength_, buf, length);
  plainTextLength_ += length;

  return true;
}

void FCGIRecord::read(int fd)
{
  clear();

  if (!getAndAssign(fd, version_, false))
    return;

  if (!getAndAssign(fd, type_, true))
    return;

  unsigned char IdB1;
  unsigned char IdB0;

  if (!getAndAssign(fd, IdB1, true))
    return;
  if (!getAndAssign(fd, IdB0, true))
    return;

  requestId_ = (IdB1 << 8) | IdB0;

  unsigned char contentLengthB1;
  unsigned char contentLengthB0;

  if (!getAndAssign(fd, contentLengthB1, true))
    return;
  if (!getAndAssign(fd, contentLengthB0, true))
    return;

  contentLength_ = (contentLengthB1 << 8) | contentLengthB0;

  if (!getAndAssign(fd, paddingLength_, true))
    return;

  if (!getAndAssign(fd, reserved_, true))
    return;

  contentData_ = new unsigned char[contentLength_];
  if (!getBuffer(fd, contentData_, contentLength_))
    return;

  unsigned char c;
  for (unsigned i = 0; i < paddingLength_; ++i)
    if (!getAndAssign(fd, c, true))
      return;

  good_ = true;
}

std::ostream& operator<< (std::ostream& o, const FCGIRecord& r)
{
  o << "version = " << (int)r.version()
    << " type = " << (int)r.type()
    << " requestId = " << (int)r.requestId()
    << " contentLength = " << r.contentLength();

  if (true || (r.type() == 5)) {
    o << " content =\n";
    for (unsigned i = 0; i < r.contentLength(); ++i)
      o << r.contentData()[i];
  }

  return o;
}

bool FCGIRecord::getParam(const std::string name, std::string& value) const
{
  for (unsigned i = 0; i < contentLength_;) {
    unsigned int nameLen;

    if ((contentData_[i] >> 7) == 0) {
      nameLen = contentData_[i];
      i += 1;
    } else {
      nameLen =
	((unsigned)(contentData_[i] & 0x7F) << 24)
	| ((unsigned)contentData_[i+1] << 16)
	| ((unsigned)contentData_[i+2] << 8)
	| ((unsigned)contentData_[i+3]);
      i += 4;
    }

    unsigned int valueLen;
  
    if ((contentData_[i] >> 7) == 0) {
      valueLen = contentData_[i];
      i += 1;
    } else {
      valueLen =
	(((unsigned)contentData_[i] & 0x7F) << 24)
	| ((unsigned)contentData_[i+1] << 16)
	| ((unsigned)contentData_[i+2] << 8)
	| ((unsigned)contentData_[i+3]);
      i += 4;
    }

    std::string thisname = std::string((char *)contentData_ + i, nameLen);
    value = std::string((char *)contentData_ + i + nameLen, valueLen);

    if (name == thisname)
      return true;

    i += nameLen + valueLen;
  }

  return false;
}
