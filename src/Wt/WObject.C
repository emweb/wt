/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WApplication"
#include "Wt/WException"
#include "Wt/WObject"
#include "Wt/WStatelessSlot"
#include "Wt/WSignal"
#include "WebUtils.h"

#include "web/WebRenderer.h"
#include "web/WebSession.h"

namespace Wt {

unsigned WObject::nextObjId_ = 0;

std::vector<WObject *> WObject::emptyObjectList_;

void WObject::seedId(unsigned id)
{
  nextObjId_ = id;
}

WObject::DeletionTracker::DeletionTracker(WObject *object)
{
#ifndef WT_TARGET_JAVA
#ifndef TRACKABLE_BROKEN
  // any signal would do, we just take one that already exists.
  connection_ = signal_.connect(boost::bind(&WObject::parent, object));
#else
  if (object->trackable_ptr_.use_count() == 0)
    object->trackable_ptr_.reset((int *)0);
  ptr = object->trackable_ptr_;
#endif
#endif
}
bool WObject::DeletionTracker::deleted() const
{
#ifndef WT_TARGET_JAVA
#ifndef TRACKABLE_BROKEN
  signal_();
  return !(connection_.connected() && !connection_.blocked());
#else
  return ptr.use_count() == 0;
#endif
#else
  return false;
#endif
}

WObject::WObject(WObject* parent)
  : statelessSlots_(0),
    id_(nextObjId_++),
    children_(0),
    parent_(parent)
#ifndef WT_CNOR
    , destroyed_(0)
#endif
{
  if (parent) {
    if (!parent->children_)
      parent->children_ = new std::vector<WObject *>;
    parent->children_->push_back(this);
  }
}

void WObject::setParent(WObject *parent)
{
  parent_ = parent;
}

bool WObject::hasParent() const
{
  return parent_;
}

void WObject::addChild(WObject *child)
{
  if (child->parent_)
    child->parent_->removeChild(child);

  if (!children_)
    children_ = new std::vector<WObject *>;

  child->setParent(this);
  children_->push_back(child);
}

void WObject::removeChild(WObject *child)
{
  if (children_) {
    Utils::erase(*children_, child);
    if (child->parent_ == this) // exception: WPopupWidget
      child->setParent(0);
  }
}

WObject::~WObject()
{
#ifndef WT_CNOR
  if (destroyed_) {
    destroyed_->emit(this);
    delete destroyed_;
  }
#endif

  for (unsigned i = 0; i < statelessSlots_.size(); ++i)
    delete statelessSlots_[i];

  if (parent_)
    Utils::erase(*parent_->children_, this);

  if (children_) {
    while (!children_->empty()) {
      WObject *c = (*children_)[0];
      if (c->parent_ == this) // exception: WPopupWidget
	c->setParent(0);
      children_->erase(children_->begin());
      delete c;
    }

    delete children_;
  }
}

const std::vector<WObject *>& WObject::children() const
{
  return children_ ? *children_ : emptyObjectList_;
}

#ifndef WT_CNOR
Signal<WObject *>& WObject::destroyed()
{
  if (!destroyed_)
    destroyed_ = new Signal<WObject *>(this);

  return *destroyed_;
}
#endif

void WObject::setObjectName(const std::string& name)
{
  // We could optimize this so that id() does not have to
  // concatenate on the fly by appending internally already the
  // uniqueId()
  name_ = name;
}

std::string WObject::objectName() const
{
  return name_;
}

const std::string WObject::uniqueId() const
{
  char buf[20];
  buf[0] = 'o';
  Utils::itoa(id_, buf + 1, 36);
  return std::string(buf);
}

const std::string WObject::id() const
{
  std::string result = objectName();

  if (!result.empty())
    return result + '_' + uniqueId();
  else
    return uniqueId();
}

void WObject::setFormData(const FormData& formData)
{ }

void WObject::setRequestTooLarge(::int64_t size)
{ }

void WObject::signalConnectionsChanged()
{ }

void WObject::resetLearnedSlots()
{
  for (unsigned i = 0; i < statelessSlots_.size(); i++)
    statelessSlots_[i]->setNotLearned();
}

void WObject::resetLearnedSlot(Method method)
{
  for (unsigned i = 0; i < statelessSlots_.size(); i++) {
    if (statelessSlots_[i]->implementsMethod(method)) {
      statelessSlots_[i]->setNotLearned();
      return;
    }
  }
}

WStatelessSlot *WObject::getStateless(Method method)
{
  return 0;
}

WStatelessSlot* WObject::isStateless(Method method)
{
  for (unsigned i = 0; i < statelessSlots_.size(); i++) {
    if (statelessSlots_[i]->implementsMethod(method))
      return statelessSlots_[i];
  }

  return getStateless(method);
}

WStatelessSlot *WObject::implementAutolearn(Method method)
{
  for (unsigned i = 0; i < statelessSlots_.size(); i++)
    if (statelessSlots_[i]->implementsMethod(method)) {
      statelessSlots_[i]->setNotLearned();
      return statelessSlots_[i];
    }

  WStatelessSlot *result = new WStatelessSlot(this, method);
  statelessSlots_.push_back(result);
  return result;
}

void WObject::isNotStateless()
{
  if (wApp) {
    wApp->session()->renderer().setStatelessSlotNotStateless();
  }
}

WStatelessSlot *WObject::implementPrelearn(Method method, Method undoMethod)
{
  for (unsigned i = 0; i < statelessSlots_.size(); i++)
    if (statelessSlots_[i]->implementsMethod(method)) {
      statelessSlots_[i]->reimplementPreLearn(undoMethod);
      return statelessSlots_[i];
    }

  WStatelessSlot *result = new WStatelessSlot(this, method, undoMethod);
  statelessSlots_.push_back(result);
  return result;
}

WStatelessSlot *WObject::implementPrelearned(Method method,
					     const std::string& jsCode)
{        
  for (unsigned i = 0; i < statelessSlots_.size(); i++)
    if (statelessSlots_[i]->implementsMethod(method)) {
      statelessSlots_[i]->reimplementJavaScript(jsCode);
      return statelessSlots_[i];
    }

  WStatelessSlot *result = new WStatelessSlot(this, method, jsCode);
  statelessSlots_.push_back(result);
  return result;
}

WObject *WObject::sender()
{
  return SignalBase::currentSender();
}

}
