/*
 * Copyright (C) 2010 Emweb bvba, Heverlee, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "CodeSession.h"

boost::recursive_mutex CodeSession::mutex_;
std::vector<CodeSession *> CodeSession::sessions_;

CodeSession::CodeSession()
  : observers_(0),
    coder_(true)
{
  generateId();

  Lock lock(mutex_);
  sessions_.push_back(this);
}

std::pair<CodeSession *, CodeSession::Connection>
  CodeSession::addObserver(const std::string& anId,
			   const BufferCallback& bufferCallback)
{
  Lock lock(mutex_);

  for (unsigned i = 0; i < sessions_.size(); ++i) {
    CodeSession *session = sessions_[i];
    if (session->id() == anId) {
      ++session->observers_;

      Connection conn = session->bufferChanged_.connect
	(boost::bind(bufferCallback, _1, _2));

      session->sessionChanged_.emit();

      return std::make_pair(session, conn);
    }
  }

  return std::make_pair((CodeSession *)0, Connection());
}

void CodeSession::removeObserver(const Connection& connection)
{
  Lock lock(mutex_);

  --observers_;
  connection.disconnect();

  sessionChanged_.emit();

  deleteIfEmpty();
}

void CodeSession::removeCoder(const Connection& connection)
{
  Lock lock(mutex_);

  coder_ = false;
  connection.disconnect();

  deleteIfEmpty();
}

void CodeSession::insertBuffer(int index)
{
  Lock lock(mutex_);

  buffers_.insert(buffers_.begin() + index, Buffer());

  bufferChanged_.emit(index, Inserted);
}

void CodeSession::updateBuffer(int buffer, const Wt::WString& name,
			       const Wt::WString& text)
{
  Lock lock(mutex_);

  buffers_[buffer].name = name;
  buffers_[buffer].text = text;

  bufferChanged_.emit(buffer, Changed);
}

std::vector<CodeSession::Buffer> CodeSession::buffers() const
{
  Lock lock(mutex_);

  return buffers_;
}

CodeSession::Buffer CodeSession::buffer(int buffer) const
{
  Lock lock(mutex_);

  return buffers_[buffer];
}

void CodeSession::deleteIfEmpty()
{
  if (observers_ == 0 && !coder_) {
    sessions_.erase(std::find(sessions_.begin(), sessions_.end(), this));

    delete this;
  }
}

void CodeSession::generateId()
{
  for (int i = 0; i < 32; ++i) {
#ifndef WIN32
    int d = lrand48() % (26 + 26 + 10);
#else
    int d = rand();
#endif

    char c = (d < 10 ? ('0' + d)
	      : (d < 36 ? ('A' + d - 10)
		 : 'a' + d - 36));

    id_.push_back(c);
  }
}
