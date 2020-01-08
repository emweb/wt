/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WApplication.h"
#include "Wt/WException.h"
#include "Wt/WObject.h"
#include "Wt/WStatelessSlot.h"
#include "Wt/WSignal.h"
#include "WebUtils.h"

#include "web/WebRenderer.h"
#include "web/WebSession.h"

namespace Wt {

unsigned WObject::nextObjId_ = 0;

void WObject::seedId(unsigned id)
{
  nextObjId_ = id;
}

WObject::WObject()
  : id_(nextObjId_++)
{ }

WObject::~WObject()
{ }

void WObject::addChild(std::unique_ptr<WObject> child)
{
  children_.push_back(std::move(child));
}

std::unique_ptr<WObject> WObject::removeChild(WObject *child)
{
  return Utils::take(children_, child);
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
  buf[0] = 'o';
  Utils::itoa(id_, buf + 1, 36);
  return std::string(buf);
}

const std::string WObject::id() const
{
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
  return nullptr;
}

WStatelessSlot* WObject::isStateless(Method method) const
{
  for (unsigned i = 0; i < statelessSlots_.size(); i++) {
    if (statelessSlots_[i]->implementsMethod(method))
      return statelessSlots_[i].get();
  }

  return (const_cast<WObject *>(this))->getStateless(method);
}

WStatelessSlot *WObject::implementAutolearn(Method method)
{
  for (unsigned i = 0; i < statelessSlots_.size(); i++)
    if (statelessSlots_[i]->implementsMethod(method)) {
      statelessSlots_[i]->setNotLearned();
      return statelessSlots_[i].get();
    }

  std::unique_ptr<WStatelessSlot> slot(new WStatelessSlot(this, method));
  auto result = slot.get();
  statelessSlots_.push_back(std::move(slot));
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
      return statelessSlots_[i].get();
    }

  std::unique_ptr<WStatelessSlot> slot
    (new WStatelessSlot(this, method, undoMethod));
  auto result = slot.get();
  statelessSlots_.push_back(std::move(slot));
  return result;
}

WStatelessSlot *WObject::implementPrelearned(Method method,
					     const std::string& jsCode)
{        
  for (unsigned i = 0; i < statelessSlots_.size(); i++)
    if (statelessSlots_[i]->implementsMethod(method)) {
      statelessSlots_[i]->reimplementJavaScript(jsCode);
      return statelessSlots_[i].get();
    }

  std::unique_ptr<WStatelessSlot> slot
    (new WStatelessSlot(this, method, jsCode));
  auto result = slot.get();
  statelessSlots_.push_back(std::move(slot));
  return result;
}

}
