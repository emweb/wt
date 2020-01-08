// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef CODE_SESSION_H_
#define CODE_SESSION_H_

#include <Wt/WString.h>
#include <Wt/WSignal.h>

#include <vector>
#include <thread>
#include <mutex>

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

  typedef std::function<void(int, BufferUpdate)> BufferCallback;
  typedef std::function<void(void)> CoderCallback;

  CodeSession(const CoderCallback& coderCallback);
  ~CodeSession();

  static void addSession(const std::shared_ptr<CodeSession> &session);

  std::string id() const { return id_; }

  static std::shared_ptr<CodeSession>
    addObserver(const std::string& id, const BufferCallback& bufferCallback);

  void removeObserver();
  void removeCoder();

  void insertBuffer(int index);
  void updateBuffer(int buffer, const Wt::WString& name,
                    const Wt::WString& text);

  std::vector<Buffer> buffers() const;
  Buffer buffer(int buffer) const;
  int observerCount() const { return observers_.size(); }

private:
  typedef std::unique_lock<std::recursive_mutex> Lock;

  struct Coder {
    std::string sessionId;
    CoderCallback callback;
  };

  struct Observer {
    std::string sessionId;
    BufferCallback callback;
  };

  std::string id_;
  std::vector<Buffer> buffers_;

  std::vector<Observer> observers_;
  std::unique_ptr<Coder> coder_;

  static std::vector<std::weak_ptr<CodeSession>> sessions_;
  static std::recursive_mutex mutex_;

  void generateId();
  static void cleanExpiredSessions();
  void postSessionChanged();
  void postBufferChanged(int buffer, BufferUpdate update);
};

#endif // CODE_SESSION_H_
