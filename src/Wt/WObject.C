/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WObject"
#include "Wt/WStatelessSlot"
#include "Wt/WSignal"
#include "Utils.h"

#include "WtException.h"

#include <stdio.h>

namespace Wt {

unsigned WObject::nextObjId_ = 0;
std::vector<WObject *> WObject::emptyObjectList_;

WObject::WObject(WObject* parent)
  : statelessSlots_(0),
    id_(nextObjId_++),
    children_(0),
    parent_(parent),
    destroyed_(0)
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

void WObject::addChild(WObject *child)
{
  if (child->parent_)
    child->parent_->removeChild(child);

  if (!children_)
    children_ = new std::vector<WObject *>;

  child->parent_ = this;
  children_->push_back(child);
}

void WObject::removeChild(WObject *child)
{
  if (child->parent_ != this)
    throw WtException("WObject::removeChild() called with non-child");

  assert(children_);
  Utils::erase(*children_, child);
  child->parent_ = 0;
}

WObject::~WObject()
{
  if (destroyed_)
    destroyed_->emit(this);

  for (unsigned i = 0; i < statelessSlots_.size(); ++i)
    delete statelessSlots_[i];

  if (parent_)
    Utils::erase(*parent_->children_, this);

  if (children_) {
    for (unsigned i = 0; i < children_->size(); ++i) {
      (*children_)[i]->parent_ = 0;
      delete (*children_)[i];
    }
    delete children_;
  }
}

const std::vector<WObject *>& WObject::children() const
{
  return children_ ? *children_ : emptyObjectList_;
}

Signal<WObject *>& WObject::destroyed()
{
  if (!destroyed_)
    destroyed_ = new Signal<WObject *>(this);

  return *destroyed_;
}

void WObject::setObjectName(const std::string& name)
{
  name_ = name;
}

std::string WObject::objectName() const
{
  return name_;
}

const std::string WObject::uniqueId() const
{
  char buf[20];
  sprintf(buf, "o%x", id_);
  return std::string(buf);
}

const std::string WObject::formName() const
{
  std::string result = objectName();

  if (!result.empty())
    return result + '-' + uniqueId();
  else
    return uniqueId();
}

void WObject::setFormData(CgiEntry *entry)
{ }

void WObject::formDataSet()
{ }

void WObject::setNoFormData()
{ }

void WObject::requestTooLarge(int size)
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

  throw WtException("WObject::resetLearnedSlot(): method that has"
		    " no stateless implementation.");
}

WStatelessSlot* WObject::isStateless(Method method)
{
  for (unsigned i = 0; i < statelessSlots_.size(); i++) {
    if (statelessSlots_[i]->implementsMethod(method))
      return statelessSlots_[i];
  }

  return 0;
}

void WObject::implementAutolearn(Method method)
{
  statelessSlots_.push_back(new WStatelessSlot(this, method));
}

void WObject::implementPrelearn(Method method, Method undoMethod)
{        
  for (unsigned i = 0; i < statelessSlots_.size(); i++)
    if (statelessSlots_[i]->implementsMethod(method)) {
      statelessSlots_[i]->reimplementPreLearn(undoMethod);
      break;
    }

  statelessSlots_.push_back(new WStatelessSlot(this, method, undoMethod));
}

void WObject::implementPrelearned(Method method, const std::string& jsCode)
{        
  for (unsigned i = 0; i < statelessSlots_.size(); i++)
    if (statelessSlots_[i]->implementsMethod(method)) {
      statelessSlots_[i]->reimplementJavaScript(jsCode);
      break;
    }

  statelessSlots_.push_back(new WStatelessSlot(this, method, jsCode));
}

WObject *WObject::sender()
{
  return SignalBase::currentSender();
}

}
