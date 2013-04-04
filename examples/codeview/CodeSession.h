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

  CodeSession(const CoderCallback& coderCallback);
  ~CodeSession();

  std::string id() const { return id_; }

  static CodeSession *
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
  typedef boost::recursive_mutex::scoped_lock Lock;

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
  Coder *coder_;

  static std::vector<CodeSession *> sessions_;
  static boost::recursive_mutex mutex_;

  void generateId();
  void deleteIfEmpty();
  void postSessionChanged();
  void postBufferChanged(int buffer, BufferUpdate update);
};

#endif // CODE_SESSION_H_
