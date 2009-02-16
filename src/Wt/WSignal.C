/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <boost/lexical_cast.hpp>
#include <stdio.h>

#include "Wt/WSignal"
#include "Wt/WApplication"
#include "Wt/WStatelessSlot"
#include "Wt/WJavaScriptSlot"

#include "WebSession.h"
#include "WtException.h"

namespace Wt {

Wt::NoClass Wt::NoClass::none;

SignalBase::~SignalBase()
{ }

void SignalBase::setBlocked(bool blocked)
{
  blocked_ = blocked;
}

void SignalBase::pushSender(WObject *sender)
{
  WebSession *sess = WebSession::instance();

  sess->pushEmitStack(sender);
}

void SignalBase::popSender()
{
  WebSession *sess = WebSession::instance();

  sess->popEmitStack();
}

WObject *SignalBase::currentSender()
{
  WebSession *sess = WebSession::instance();

  return sess->emitStackTop();
}

Signal<void>::Signal(WObject *sender)
  : Signal<>(sender)
{ }

EventSignalBase
::StatelessConnection::StatelessConnection(const boost::signals::connection& c,
                                           WObject *t,
                                           WStatelessSlot *s)
  : connection(c),
    target(t),
    slot(s)
{ }

bool EventSignalBase::StatelessConnection::ok() const
{
  return target == 0 || connection.connected();
}

EventSignalBase::Impl::Impl()
  : id_(-1),
    dynamic_(0)
{ }

int EventSignalBase::Impl::nextId_ = 0;

void EventSignalBase::createImpl() const
{
  if (!impl_)
    impl_ = new Impl();
}

bool EventSignalBase::needUpdate() const
{
  return impl_ && impl_->flags_.test(Impl::BIT_NEED_UPDATE);
}

void EventSignalBase::updateOk()
{
  if (impl_)
    impl_->flags_.set(Impl::BIT_NEED_UPDATE, false);
}

void EventSignalBase::removeSlot(WStatelessSlot *s)
{
  if (impl_) {
    for (unsigned i = 0; i < impl_->connections_.size(); ++i) {
      if (impl_->connections_[i].slot == s) {
	impl_->connections_.erase(impl_->connections_.begin() + i);
	senderRepaint();
	return;
      }
    }
  }

  assert(false);
}

const std::string EventSignalBase::encodeCmd() const
{
  createImpl();

  if (impl_->id_ == -1)
    impl_->id_ = Impl::nextId_++;

  char buf[20];
  sprintf(buf, "s%x", impl_->id_);
  return std::string(buf);
}

const std::string
EventSignalBase::createUserEventCall(const std::string& jsObject,
				     const std::string& jsEvent,
				     const std::string& eventName,
				     const std::string& arg1,
				     const std::string& arg2,
				     const std::string& arg3,
				     const std::string& arg4,
				     const std::string& arg5,
				     const std::string& arg6) const
{
  std::stringstream result;

  result << javaScript();

  if (isExposedSignal()) {
    WApplication *app = WApplication::instance();

    result << app->javaScriptClass() << ".emit('"
	   << sender()->uniqueId();

    if (!jsObject.empty())
      result << "', { name:'" << eventName << "', eventObject:" << jsObject
	     << ", event:" << jsEvent << "}";
    else
      result << "','" << eventName << "'";

    if (!arg1.empty()) {
      result << "," << arg1;
      if (!arg2.empty()) {
	result << "," << arg2;
	if (!arg3.empty()) {
	  result << "," << arg3;
	  if (!arg4.empty()) {
	    result << "," << arg4;
	    if (!arg5.empty()) {
	      result << "," << arg5;
	      if (!arg6.empty()) {
		result << "," << arg6;
	      }
	    }
	  }
	}
      }
    }

    result << ");";
  }

  return result.str();
}

const std::string EventSignalBase::javaScript() const
{
  std::string result = "";

  if (impl_) {
    for (unsigned i = 0; i < impl_->connections_.size(); ++i) {
      if (impl_->connections_[i].ok()) {
	if (impl_->connections_[i].slot->learned())
	  result += impl_->connections_[i].slot->javaScript();
      }
    }

    if (preventDefault())
      result += WT_CLASS ".cancelEvent(e);";
  }

  return result;
}

void EventSignalBase::setNotExposed()
{
  if (impl_)
    impl_->flags_.reset(Impl::BIT_EXPOSED);
}

bool EventSignalBase::isExposedSignal() const
{
  if (impl_)
    return impl_->flags_.test(Impl::BIT_EXPOSED);
  else
    return false;
}

void EventSignalBase::setPreventDefault(bool prevent)
{
  if (preventDefault() != prevent) {
    if (prevent)
      createImpl();

    impl_->flags_.set(Impl::BIT_PREVENT_DEFAULT, prevent);
    senderRepaint();
  }
}

bool EventSignalBase::preventDefault() const
{
  if (impl_)
    return impl_->flags_.test(Impl::BIT_PREVENT_DEFAULT);
  else
    return false;
}

void EventSignalBase::prepareDestruct()
{
  // uses virtual method encodeCmd()

  if (impl_) {
    if (impl_->flags_.test(Impl::BIT_NEEDS_AUTOLEARN)) {
      WApplication *app = WApplication::instance();
      if (app)
	app->removeExposedSignal(this);  
    }
    impl_->flags_.reset(Impl::BIT_NEEDS_AUTOLEARN);
  }
}

EventSignalBase::~EventSignalBase()
{
  prepareDestruct();

  if (impl_) {
    for (unsigned i = 0; i < impl_->connections_.size(); ++i) {
      if (impl_->connections_[i].ok()) {
	impl_->connections_[i].slot->removeConnection(this);
      }
    }

    delete impl_;
  }
}

boost::signals::connection EventSignalBase::connect(WObject::Method method,
                                                    WObject *target,
                                                    WStatelessSlot *slot)
{
  createImpl();

  boost::signals::connection c
    = impl_->dummy_.connect(boost::bind(method, target));

  slot->addConnection(this);

  impl_->connections_.push_back(StatelessConnection(c, target, slot));

  senderRepaint();

  return c;
}

void EventSignalBase::connect(JSlot& slot)
{
  WStatelessSlot *s = slot.slotimp();
  s->addConnection(this);

  createImpl();

  boost::signals::connection c;

  impl_->connections_.push_back(StatelessConnection(c, 0, s));
}

bool EventSignalBase::isConnected() const
{
  if (!impl_)
    return false;

  bool result = impl_->dummy_.num_slots() > 0;

  if (!result) {
    for (unsigned i = 0; i < impl_->connections_.size(); ++i) {
      if (impl_->connections_[i].target == 0)
	return true;
    }
  }

  return result;
}

void EventSignalBase::exposeSignal()
{
  // cheap catch: if it's exposed, for sure it is also autolearn
  if (impl_ && impl_->flags_.test(Impl::BIT_EXPOSED)) {
    senderRepaint();
    return;
  }

  WApplication *app = WApplication::instance();
  app->addExposedSignal(this);

  createImpl();

  impl_->flags_.set(Impl::BIT_NEEDS_AUTOLEARN);

  if (app->exposeSignals())
    impl_->flags_.set(Impl::BIT_EXPOSED);

  senderRepaint();
}

void EventSignalBase::senderRepaint()
{
  assert(impl_);

  impl_->flags_.set(Impl::BIT_NEED_UPDATE, true);
  sender()->signalConnectionsChanged();
}

void EventSignalBase::processNonLearnedStateless()
{
  if (impl_) {
    std::vector<StatelessConnection> copy = impl_->connections_;

    for (unsigned i = 0; i < copy.size(); ++i) {
      StatelessConnection& c = copy[i];

      if (c.ok() && !c.slot->learned())
	c.slot->trigger();
    }
  }
}

void EventSignalBase::processLearnedStateless()
{
  if (impl_) {
    std::vector<StatelessConnection> copy = impl_->connections_;

    for (unsigned i = 0; i < copy.size(); ++i) {
      StatelessConnection& c = copy[i];

      if (c.ok() && c.slot->learned())
	c.slot->trigger();
    }
  }
}

void EventSignalBase::processPreLearnStateless(SlotLearnerInterface *learner)
{
  if (impl_) {
    std::vector<StatelessConnection> copy = impl_->connections_;

    for (unsigned i = 0; i < copy.size(); ++i) {
      StatelessConnection& c = copy[i];

      if (c.ok()
	  && !c.slot->learned()
	  && c.slot->type() == WStatelessSlot::PreLearnStateless) {
	learner->learn(c.slot);
      }
    }
  }
}

void EventSignalBase::processAutoLearnStateless(SlotLearnerInterface *learner)
{
  if (impl_) {
    bool changed = false;

    std::vector<StatelessConnection> copy = impl_->connections_;

    for (unsigned i = 0; i < copy.size(); ++i) {
      StatelessConnection& c = copy[i];

      if (c.ok()
	  && !c.slot->learned()
	  && c.slot->type() == WStatelessSlot::AutoLearnStateless) {
	learner->learn(c.slot);
	changed = true;
      }
    }

    if (changed)
      senderRepaint();
  }
}

EventSignal<void>::EventSignal(WObject *sender)
  : EventSignalBase(sender),
    relay_(0)
{ }

EventSignal<void>::EventSignal(EventSignalBase *relay)
  : EventSignalBase(relay ? relay->sender() : 0),
    relay_(relay)
{ }

void EventSignal<void>::setRelay(EventSignalBase *relay)
{
  relay_ = relay;
}

EventSignal<void>::~EventSignal()
{
  if (impl_) {
    BoostSignalType *dynamic = (BoostSignalType *)impl_->dynamic_;
    delete dynamic;
  }
}

bool EventSignal<void>::isConnected() const
{
  if (relay_)
    return relay_->isConnected();
  else {
    if (EventSignalBase::isConnected())
      return true;

    if (impl_) {
      BoostSignalType *dynamic = (BoostSignalType *)impl_->dynamic_;

      return dynamic && dynamic->num_slots() > 0;
    } else
      return false;
  }
}

void EventSignal<void>::connect(JSlot& slot)
{
  if (relay_)
    relay_->connect(slot);
  else
    EventSignalBase::connect(slot);
}

boost::signals::connection EventSignal<void>
::connectBase(WObject *target, WObject::Method method)
{
  if (relay_)
    return relay_->connectBase(target, method);
  else
    return connect(target, method);
}

void EventSignal<void>::emit()
{
  if (isBlocked())
    return;

  if (relay_)
    throw WtException("Cannot emit a relayed signal");

  pushSender(sender());

  processLearnedStateless();
  processNonLearnedStateless();

  if (impl_) {
    BoostSignalType *dynamic = (BoostSignalType *)impl_->dynamic_;
    if (dynamic)
      (*dynamic)();
  }

  popSender();
}

void EventSignal<void>::operator()()
{
  emit();
}

void EventSignal<void>::processDynamic(const JavaScriptEvent& jse)
{
  if (relay_)
    throw WtException("Internal error: processDynamic on a relayed signal?");
  
  processNonLearnedStateless();

  if (impl_) {
    BoostSignalType *dynamic = (BoostSignalType *)impl_->dynamic_;
    if (dynamic) {
      pushSender(sender());
      (*dynamic)();
      popSender();
    }
  }
}

}
