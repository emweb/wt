// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WSTATELESSSLOT_H_
#define WSTATELESSSLOT_H_

#include <Wt/WDllDefs.h>
#include <vector>
#include <string>
#include <Wt/WObject.h>

namespace Wt {

class WObject;
class JSlot;
class EventSignalBase;

class WT_API WStatelessSlot
{
public:
  WStatelessSlot(const std::string& javaScript);
  ~WStatelessSlot();

  typedef void (WObject::*WObjectMethod)();

  enum class SlotType { 
    AutoLearnStateless,
    PreLearnStateless,
    JavaScriptSpecified
  };
    
  SlotType type() const; 
  bool learned() const;
  void setNotLearned();
  void invalidate();
  bool invalidated() const;
  void setJavaScript(const std::string& javaScript);

  bool addConnection(EventSignalBase *);
  bool removeConnection(EventSignalBase *);

  const std::string& javaScript() const { return jscript_; }

  void trigger();
  void undoTrigger();

  bool implementsMethod(WObjectMethod method) const;

#ifndef WT_TARGET_JAVA
  void disconnectFrom(EventSignalBase *);
#endif

protected:
  WStatelessSlot(WObject *target, WObjectMethod method);
  WStatelessSlot(WObject *target, WObjectMethod method,
		 WObjectMethod undoMethod);
  WStatelessSlot(WObject *target, WObjectMethod method,
		 const std::string& javaScript);

private:
  void reimplementPreLearn(WObjectMethod undoMethod);
  void reimplementJavaScript(const std::string& javaScript);

  WObject       *target_;
  WObjectMethod  method_;
  WObjectMethod  undoMethod_;

  bool                           learned_;
  std::string                    jscript_;
  std::vector<EventSignalBase *> connectingSignals_;     

  friend class WObject;
  friend class JSlot;
};

class WT_API SlotLearnerInterface {
public:
  virtual std::string learn(WStatelessSlot* slot) = 0;
  virtual ~SlotLearnerInterface() { }
};

}

#endif // WSTATELESSSLOT_H_
