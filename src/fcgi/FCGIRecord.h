/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
// This may look like C code, but it's really -*- C++ -*-
#ifndef FCGIRECORD_H_
#define FCGIRECORD_H_

#include <stdio.h>
#include <iostream>

class FCGIRecord
{
public:
  FCGIRecord();
  FCGIRecord(short requestId, char version);
  ~FCGIRecord();

  void clear();
  bool good() { return good_; }

  unsigned char type() const { return type_; }
  unsigned char version() const { return version_; }
  unsigned short requestId() const { return requestId_; }
  unsigned int contentLength() const { return contentLength_; }
  const unsigned char *contentData() const { return contentData_; }

  unsigned int plainTextLength() const { return plainTextLength_; }
  const unsigned char *plainText() const { return plainTextBuf_; }

  void read(int fd);

  bool getParam(const std::string name, std::string& value) const;

private:
  bool good_;

  unsigned char version_;
  unsigned char type_;
  unsigned short requestId_;
  unsigned int contentLength_;
  unsigned char paddingLength_;
  unsigned char reserved_;
  unsigned char *contentData_;

  unsigned int plainTextLength_;
  unsigned char *plainTextBuf_;
  unsigned int plainTextBufLength_;

  int getChar(int fd, bool waitForIt);
  bool getBuffer(int fd, unsigned char *buf, int length);
  bool getAndAssign(int fd, unsigned char& r, bool waitForIt);
};

extern std::ostream& operator<< (std::ostream&, const FCGIRecord& r);

#endif // FCGIRECORD_H_
