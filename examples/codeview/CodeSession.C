/*
 * Copyright (C) 2010 Emweb bvba, Heverlee, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WApplication>
#include <Wt/WServer>
#include <Wt/WRandom>

#include "CodeSession.h"

boost::recursive_mutex CodeSession::mutex_;
std::vector<CodeSession *> CodeSession::sessions_;

CodeSession::CodeSession(const CoderCallback& coderCallback)
{
  generateId();

  coder_ = new Coder();
  coder_->sessionId = Wt::WApplication::instance()->sessionId();
  coder_->callback = coderCallback;

  Lock lock(mutex_);
  sessions_.push_back(this);
}

CodeSession::~CodeSession()
{
  delete coder_;
}

CodeSession * CodeSession::addObserver(const std::string& id,
				       const BufferCallback& bufferCallback)
{
  Lock lock(mutex_);

  for (unsigned i = 0; i < sessions_.size(); ++i) {
    CodeSession *session = sessions_[i];
    if (session->id() == id) {
      Observer observer;
      observer.sessionId = Wt::WApplication::instance()->sessionId();
      observer.callback = bufferCallback;

      session->observers_.push_back(observer);
      session->postSessionChanged();

      return session;
    }
  }

  return 0;
}

void CodeSession::removeObserver()
{
  Lock lock(mutex_);

  std::string sessionId = Wt::WApplication::instance()->sessionId();

  for (unsigned i = 0; i < observers_.size(); ++i) {
    if (observers_[i].sessionId == sessionId) {
      observers_.erase(observers_.begin() + i);

      postSessionChanged();

      deleteIfEmpty();
      return;
    }
  }
}

void CodeSession::removeCoder()
{
  Lock lock(mutex_);

  delete coder_;
  coder_ = 0;

  deleteIfEmpty();
}

void CodeSession::insertBuffer(int index)
{
  Lock lock(mutex_);

  buffers_.insert(buffers_.begin() + index, Buffer());

  postBufferChanged(index, Inserted);
}

void CodeSession::updateBuffer(int buffer, const Wt::WString& name,
			       const Wt::WString& text)
{
  Lock lock(mutex_);

  buffers_[buffer].name = name;
  buffers_[buffer].text = text;

  postBufferChanged(buffer, Changed);
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
  if (observers_.size() == 0 && !coder_) {
    sessions_.erase(std::find(sessions_.begin(), sessions_.end(), this));

    delete this;
  }
}

void CodeSession::generateId()
{
  id_ = Wt::WRandom::generateId(32);
}

void CodeSession::postSessionChanged()
{
  if (coder_)
    Wt::WServer::instance()->post(coder_->sessionId, coder_->callback);
}

void CodeSession::postBufferChanged(int buffer, BufferUpdate update)
{
  for (unsigned i = 0; i < observers_.size(); ++i) {
    Observer& observer = observers_[i];
    Wt::WServer::instance()
      ->post(observer.sessionId,
	     boost::bind(observer.callback, buffer, update));
  }
}
