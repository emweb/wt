// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2010 Emweb bvba, Heverlee, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef CODE_SESSION_H_
#define CODE_SESSION_H_

#include <Wt/WString>
#include <Wt/WSignal>
#include <Wt/SyncLock>

#include <vector>
#include <boost/thread.hpp>

class CodeSession
{
public:
  enum BufferUpdate {
    Inserted,
    Deleted,
    Changed
  };

  struct Buffer {
    Wt::WString name;
    Wt::WString text;
  };

  typedef boost::function<void(int, BufferUpdate)> BufferCallback;
  typedef boost::function<void(void)> CoderCallback;
  typedef boost::signals::connection Connection;

  CodeSession();

  std::string id() const { return id_; }

  static std::pair<CodeSession *, Connection>
    addObserver(const std::string& anId, const BufferCallback& bufferCallback);

  void removeObserver(const Connection& connection);
  void removeCoder(const Connection& connection);

  void insertBuffer(int index);
  void updateBuffer(int buffer, const Wt::WString& name, const Wt::WString& text);

  Wt::Signal<>& sessionChanged() { return sessionChanged_; }

  std::vector<Buffer> buffers() const;
  Buffer buffer(int buffer) const;
  int observerCount() const { return observers_; }

private:
  typedef Wt::SyncLock<boost::recursive_mutex::scoped_lock> Lock;

  Wt::Signal<int, BufferUpdate> bufferChanged_;
  Wt::Signal<> sessionChanged_;

  std::string id_;
  std::vector<Buffer> buffers_;
  int observers_;
  bool coder_;

  static std::vector<CodeSession *> sessions_;
  static boost::recursive_mutex mutex_;

  void generateId();
  void deleteIfEmpty();
};

#endif // CODE_SESSION_H_
