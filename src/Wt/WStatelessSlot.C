#include "Wt/WStatelessSlot"
#include "Wt/WSignal"

#include <string>
#include "WebUtils.h"

namespace Wt {

WStatelessSlot::WStatelessSlot(WObject* obj, WObjectMethod method)
  : target_(obj),
    method_(method),
    undoMethod_(0),
    learned_(false)
{ }

WStatelessSlot::WStatelessSlot(WObject* obj, WObjectMethod method, 
			       WObjectMethod undomethod)
  : target_(obj),
    method_(method),
    undoMethod_(undomethod),
    learned_(false)
{ }

WStatelessSlot::WStatelessSlot(WObject* obj, WObjectMethod method,
			       const std::string& javaScript)
  : target_(obj),
    method_(method),
    undoMethod_(0),
    learned_(true),
    jscript_(javaScript)
{ }

WStatelessSlot::WStatelessSlot(const std::string& javaScript)
  : target_(0),
    method_(0),
    undoMethod_(0),
    learned_(true),
    jscript_(javaScript)
{ }

WStatelessSlot::~WStatelessSlot()
{
  for (unsigned i = 0; i < connectingSignals_.size(); ++i)
    connectingSignals_[i]->removeSlot(this);
}

#ifndef WT_TARGET_JAVA
void WStatelessSlot::disconnectFrom(EventSignalBase *signal)
{
  for (std::vector<EventSignalBase *>::iterator it = connectingSignals_.begin(); it != connectingSignals_.end(); ++it) {
    if (signal == *it) {
      connectingSignals_.erase(it);
      signal->removeSlot(this);
      break;
    }
  }
}
#endif

bool WStatelessSlot::implementsMethod(WObjectMethod method) const
{
  return method_ == method;
}

void WStatelessSlot::reimplementPreLearn(WObjectMethod undoMethod)
{
  undoMethod_ = undoMethod;
  setNotLearned();
}

void WStatelessSlot::reimplementJavaScript(const std::string& javaScript)
{
  undoMethod_ = 0;
  learned_ = true;
  setJavaScript(javaScript);
}

WStatelessSlot::SlotType WStatelessSlot::type() const
{
  if (method_ == 0)
    return JavaScriptSpecified;
  else
    if (undoMethod_ == 0)
      return AutoLearnStateless;
    else
      return PreLearnStateless;
} 
 
bool WStatelessSlot::learned() const
{
  return learned_;
}

bool WStatelessSlot::invalidated() const
{
  return !learned_ && !method_;
}

void WStatelessSlot::setJavaScript(const std::string& javaScript)
{
  jscript_ = javaScript;
  learned_ = true;

  for (size_t i = 0; i < connectingSignals_.size(); i++)
    connectingSignals_[i]->senderRepaint();
}

void WStatelessSlot::invalidate()
{
  /* Is not actually a stateless slot, see WObject::isNotStateless() */
  setNotLearned();
  method_ = 0;
}

void WStatelessSlot::setNotLearned()
{
  if (learned_) {
    jscript_.clear();
    learned_ = false;

    for (size_t i = 0; i < connectingSignals_.size(); i++)
      connectingSignals_[i]->senderRepaint();    
  }
}

void WStatelessSlot::trigger()
{
  if (method_)
    (target_->*(method_))();
}

void WStatelessSlot::undoTrigger()
{
  if (undoMethod_)
    (target_->*(undoMethod_))();
}

bool WStatelessSlot::addConnection(EventSignalBase* s)
{
  int f = Utils::indexOf(connectingSignals_, s);
  if (f == -1) {
    connectingSignals_.push_back(s);
    return true;
  } else
    return false;
}	

bool WStatelessSlot::removeConnection(EventSignalBase* s)	
{
  return Utils::erase(connectingSignals_, s);
}

}
