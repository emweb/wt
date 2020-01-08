/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WApplication.h>
#include <Wt/WServer.h>
#include <Wt/WRandom.h>
#include <mutex>

#include "CodeSession.h"

std::recursive_mutex CodeSession::mutex_;
std::vector<std::weak_ptr<CodeSession>> CodeSession::sessions_;

CodeSession::CodeSession(const CoderCallback& coderCallback)
{
  generateId();

  coder_ = Wt::cpp14::make_unique<Coder>();
  coder_->sessionId = Wt::WApplication::instance()->sessionId();
  coder_->callback = coderCallback;
}

CodeSession::~CodeSession()
{
}

void CodeSession::addSession(const std::shared_ptr<CodeSession> &session)
{
  Lock lock(mutex_);
  cleanExpiredSessions();
  sessions_.push_back(session);
}

std::shared_ptr<CodeSession> CodeSession::addObserver(const std::string& id,
                                       const BufferCallback& bufferCallback)
{
  Lock lock(mutex_);

  for (auto& sessionPtr : sessions_) {
    auto session = sessionPtr.lock();
    if (session && session->id() == id) {
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

      return;
    }
  }
}

void CodeSession::removeCoder()
{
  Lock lock(mutex_);
  coder_.reset();
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

void CodeSession::cleanExpiredSessions()
{
  auto it = sessions_.begin();

  while(it != sessions_.end()) {
    if((*it).expired()) {
      it = sessions_.erase(it);
    }
    else ++it;
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
  for (auto& observer : observers_) {
    Wt::WServer::instance()
      ->post(observer.sessionId,
             std::bind(observer.callback, buffer, update));
  }
}
