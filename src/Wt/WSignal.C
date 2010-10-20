/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <boost/lexical_cast.hpp>
#include <boost/pool/pool.hpp>
#include <stdio.h>

#include "Wt/WSignal"
#include "Wt/WApplication"
#include "Wt/WStatelessSlot"
#include "Wt/WJavaScriptSlot"

#include "Utils.h"
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
  if (sess) {
    sess->pushEmitStack(sender);
  }
}

void SignalBase::popSender()
{
  WebSession *sess = WebSession::instance();
  if (sess) {
    sess->popEmitStack();
  }
}

WObject *SignalBase::currentSender()
{
  WebSession *sess = WebSession::instance();
  if (sess) {
    return sess->emitStackTop();
  } else {
    return 0;
  }
}

#ifndef WT_CNOR
Signal<void>::Signal(WObject *sender)
  : Signal<>(sender)
{ }
#endif // WT_CNOR

void *EventSignalBase::alloc()
{
  return WApplication::instance()->eventSignalPool_->malloc();
}

void EventSignalBase::free(void *s)
{
  if (s)
    WApplication::instance()->eventSignalPool_->free(s);
}

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

int EventSignalBase::nextId_ = 0;

bool EventSignalBase::needsUpdate(bool all) const
{
  return (!all && flags_.test(BIT_NEED_UPDATE))
    || (all &&
	(isConnected() || defaultActionPrevented() || propagationPrevented()));
}

void EventSignalBase::updateOk()
{
  flags_.set(BIT_NEED_UPDATE, false);
}

void EventSignalBase::removeSlot(WStatelessSlot *s)
{
  for (unsigned i = 0; i < connections_.size(); ++i) {
    if (connections_[i].slot == s) {
      connections_.erase(connections_.begin() + i);
      senderRepaint();
      return;
    }
  }

  assert(false);
}

const std::string EventSignalBase::encodeCmd() const
{
  char buf[20];
  buf[0] = 's';
  Utils::itoa(id_, buf + 1, 16);
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

  for (unsigned i = 0; i < connections_.size(); ++i) {
    if (connections_[i].ok()) {
      if (connections_[i].slot->learned())
	result += connections_[i].slot->javaScript();
    }
  }

  if (defaultActionPrevented() || propagationPrevented()) {
    result += WT_CLASS ".cancelEvent(e";
    if (defaultActionPrevented() && propagationPrevented())
      result += ");";
    else if (defaultActionPrevented())
      result += ",0x2);";
    else
      result += ",0x1);";
  }

  return result;
}

void EventSignalBase::setNotExposed()
{
  flags_.reset(BIT_EXPOSED);
}

void EventSignalBase::disconnect(boost::signals::connection& conn)
{
  conn.disconnect();

  if (flags_.test(BIT_NEEDS_AUTOLEARN))
    if (!isConnected()) {
      WApplication *app = WApplication::instance();
      app->removeExposedSignal(this);
      flags_.reset(BIT_NEEDS_AUTOLEARN);
      setNotExposed();
    }

  senderRepaint();
}

bool EventSignalBase::isExposedSignal() const
{
  return flags_.test(BIT_EXPOSED);
}

void EventSignalBase::preventDefaultAction(bool prevent)
{
  if (defaultActionPrevented() != prevent) {
    flags_.set(BIT_PREVENT_DEFAULT, prevent);
    senderRepaint();
  }
}

bool EventSignalBase::defaultActionPrevented() const
{
  return flags_.test(BIT_PREVENT_DEFAULT);
}

void EventSignalBase::preventPropagation(bool prevent)
{
  if (propagationPrevented() != prevent) {
    flags_.set(BIT_PREVENT_PROPAGATION, prevent);
    senderRepaint();
  }
}

bool EventSignalBase::propagationPrevented() const
{
  return flags_.test(BIT_PREVENT_PROPAGATION);
}

void EventSignalBase::prepareDestruct()
{
  // uses virtual method encodeCmd()
  if (flags_.test(BIT_NEEDS_AUTOLEARN)) {
    WApplication *app = WApplication::instance();
    if (app)
      app->removeExposedSignal(this);
    flags_.reset(BIT_NEEDS_AUTOLEARN);
  }
}

EventSignalBase::~EventSignalBase()
{
  prepareDestruct();

  for (unsigned i = 0; i < connections_.size(); ++i) {
    if (connections_[i].ok())
      if (!connections_[i].slot->removeConnection(this))
	delete connections_[i].slot;
  }
}

#ifndef WT_CNOR
boost::signals::connection
EventSignalBase::connectStateless(WObject::Method method,
				  WObject *target,
				  WStatelessSlot *slot)
{
  boost::signals::connection c = dummy_.connect(boost::bind(method, target));
  slot->addConnection(this);
  connections_.push_back(StatelessConnection(c, target, slot));

  senderRepaint();

  return c;
}
#endif // WT_CNOR

void EventSignalBase::connect(JSlot& slot)
{
  WStatelessSlot *s = slot.slotimp();

  if (s->addConnection(this)) {
    boost::signals::connection c;
    connections_.push_back(StatelessConnection(c, 0, s));

    senderRepaint();
  }
}

void EventSignalBase::connect(const std::string& javaScript)
{
  boost::signals::connection c;
  connections_.push_back
    (StatelessConnection(c, 0,
			 new WStatelessSlot("(" + javaScript  + ")(o,e);")));

  senderRepaint();
}

#ifndef WT_CNOR
bool EventSignalBase::isConnected() const
{
  bool result = dummy_.num_slots() > 0;

  if (!result) {
    for (unsigned i = 0; i < connections_.size(); ++i) {
      if (connections_[i].target == 0)
	return true;
    }
  }

  return result;
}
#endif // WT_CNOR

void EventSignalBase::exposeSignal()
{
  /*
   * - BIT_EXPOSED indicates whether the signal invokes a server-side event
   * - BIT_AUTOLEARN indicates whether the signal is in the WApplication's
   *   exposed signals list, which is used as a list of signals that require
   *   stateless slot learning.
   *
   * The difference is only signals in those widgets that are used to
   * render a WViewWidget: they are not exposed but need learning
   */

  // cheap catch: if it's exposed, for sure it is also autolearn
  if (flags_.test(BIT_EXPOSED)) {
    senderRepaint();
    return;
  }

  WApplication *app = WApplication::instance();
  app->addExposedSignal(this);

  flags_.set(BIT_NEEDS_AUTOLEARN);

  if (app->exposeSignals())
    flags_.set(BIT_EXPOSED);

  senderRepaint();
}

void EventSignalBase::senderRepaint()
{
  flags_.set(BIT_NEED_UPDATE, true);
  sender()->signalConnectionsChanged();
}

void EventSignalBase::processNonLearnedStateless() const
{
  std::vector<StatelessConnection> copy = connections_;

  for (unsigned i = 0; i < copy.size(); ++i) {
    StatelessConnection& c = copy[i];

    if (c.ok() && !c.slot->learned())
      c.slot->trigger();
  }
}

void EventSignalBase::processLearnedStateless() const
{
  std::vector<StatelessConnection> copy = connections_;

  for (unsigned i = 0; i < copy.size(); ++i) {
    StatelessConnection& c = copy[i];

    if (c.ok() && c.slot->learned())
      c.slot->trigger();
  }
}

void EventSignalBase::processPreLearnStateless(SlotLearnerInterface *learner)
{
  std::vector<StatelessConnection> copy = connections_;

  for (unsigned i = 0; i < copy.size(); ++i) {
    StatelessConnection& c = copy[i];

    if (c.ok()
	&& !c.slot->learned()
	&& c.slot->type() == WStatelessSlot::PreLearnStateless) {
      learner->learn(c.slot);
    }
  }
}

void EventSignalBase::processAutoLearnStateless(SlotLearnerInterface *learner)
{
  bool changed = false;

  std::vector<StatelessConnection> copy = connections_;

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
