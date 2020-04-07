/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <boost/pool/pool.hpp>

#include "Wt/WSignal.h"
#include "Wt/WApplication.h"
#include "Wt/WStatelessSlot.h"
#include "Wt/WJavaScriptSlot.h"

#include "WebUtils.h"
#include "WebSession.h"

namespace Wt {

SignalBase::SignalBase()
{ }

SignalBase::~SignalBase()
{ }

EventSignalBase::EventSignalBase(const char *name, WObject *owner,
				 bool autoLearn)
  : name_(name), owner_(owner), id_(nextId_++)
{
  if (!name_)
    flags_.set(BIT_SIGNAL_SERVER_ANYWAY);

  if (autoLearn)
    flags_.set(BIT_CAN_AUTOLEARN); // requires sender is a WWidget !
}

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
::StatelessConnection::StatelessConnection(const Wt::Signals::connection& c,
                                           WObject *t,
                                           WStatelessSlot *s)
  : connection(c),
    target(t),
    slot(s)
{ }

bool EventSignalBase::StatelessConnection::ok() const
{
  return target == nullptr || connection.isConnected();
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
      ownerRepaint();
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
				     std::initializer_list<std::string> args)
  const
{
  /*
   * If we aren't connected yet to anything, assume we will be later to
   * a server-side signal, and expose the signal now.
   */
  if (!this->isExposedSignal() && !isConnected())
    const_cast<EventSignalBase*>(this)->exposeSignal();

  WStringStream result;
  int i = 0;

  for (const std::string& a : args) {
    if (++i == 1)
      result << "var a";
    else
      result << ",a";
    result << i << "=" << a;
  }

  if (i != 0)
    result << ";";

  result << javaScript();

  if (flags_.test(BIT_SERVER_EVENT)) {
    WApplication *app = WApplication::instance();

    std::string senderId = encodeCmd();
    senderId = senderId.substr(0, senderId.length() - eventName.length() - 1);

    result << app->javaScriptClass() << ".emit('"
	   << senderId;

    if (!jsObject.empty())
      result << "', { name:'" << eventName << "', eventObject:" << jsObject
	     << ", event:" << jsEvent << "}";
    else
      result << "','" << eventName << "'";

    for (const std::string& a : args)
      result << "," << a;

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
  flags_.reset(BIT_SERVER_EVENT);
  flags_.reset(BIT_SIGNAL_SERVER_ANYWAY);
}

#ifndef WT_TARGET_JAVA
void EventSignalBase::disconnect(JSlot &slot)
{
  slot.disconnectFrom(this);
}
#endif

void EventSignalBase::disconnect(Wt::Signals::connection& conn)
{
  conn.disconnect();

  if (flags_.test(BIT_EXPOSED))
    if (!isConnected()) {
      WApplication *app = WApplication::instance();
      app->removeExposedSignal(this);
      flags_.reset(BIT_EXPOSED);
      setNotExposed();
    }

  ownerRepaint();
}

bool EventSignalBase::isExposedSignal() const
{
  return flags_.test(BIT_SERVER_EVENT);
}

bool EventSignalBase::canAutoLearn() const
{
  return flags_.test(BIT_CAN_AUTOLEARN);
}

void EventSignalBase::preventDefaultAction(bool prevent)
{
  if (defaultActionPrevented() != prevent) {
    flags_.set(BIT_PREVENT_DEFAULT, prevent);
    ownerRepaint();
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
    ownerRepaint();
  }
}

bool EventSignalBase::propagationPrevented() const
{
  return flags_.test(BIT_PREVENT_PROPAGATION);
}

void EventSignalBase::prepareDestruct()
{
  // uses virtual method encodeCmd()
  if (flags_.test(BIT_EXPOSED)) {
    WApplication *app = WApplication::instance();
    if (app)
      app->removeExposedSignal(this);
    flags_.reset(BIT_EXPOSED);
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
Wt::Signals::connection
EventSignalBase::connectStateless(WObject::Method method,
				  WObject *target,
				  WStatelessSlot *slot)
{
  Wt::Signals::connection c
    = dummy_.connect(std::bind(method, target), target);
  if (slot->addConnection(this))
    connections_.push_back(StatelessConnection(c, target, slot));

  ownerRepaint();

  return c;
}
#endif // WT_CNOR

void EventSignalBase::connect(JSlot& slot)
{
  WStatelessSlot *s = slot.slotimp();

  if (s->addConnection(this)) {
    Wt::Signals::connection c;
    connections_.push_back(StatelessConnection(c, nullptr, s));

    ownerRepaint();
  }
}

void EventSignalBase::connect(const std::string& javaScript)
{
  Wt::Signals::connection c;

  int argc = argumentCount(); // user arguments, excluding 'e'

  WStringStream ss;
  ss << "(" << javaScript << ")(o,e";
  for (int i = 0; i < argc; ++i)
    ss << ",a" << (i+1);
  ss << ");";

  connections_.push_back
    (StatelessConnection(c, nullptr, new WStatelessSlot(ss.str())));

  ownerRepaint();
}

#ifndef WT_CNOR
bool EventSignalBase::isConnected() const
{
  bool result = dummy_.isConnected();

  if (!result) {
    for (unsigned i = 0; i < connections_.size(); ++i) {
      if (connections_[i].target == nullptr)
	return true;
    }
  }

  return result;
}
#endif // WT_CNOR

void EventSignalBase::exposeSignal()
{
  /*
   * - BIT_SERVER_EVENT indicates whether the signal invokes a server-side event
   * - BIT_EXPOSED indicates whether the signal is in the WApplication's
   *   exposed signals list, which is used as a list of signals that require
   *   stateless slot learning (if BIT_AUTOLEARN).
   *
   * The difference is only signals in those widgets that are used to
   * render a WViewWidget: they are not exposed but need learning
   */

  // cheap catch: if it generates a server event, for sure it is also exposed
  if (flags_.test(BIT_SERVER_EVENT)) {
    ownerRepaint();
    return;
  }

  WApplication *app = WApplication::instance();

  app->addExposedSignal(this);

  flags_.set(BIT_EXPOSED);

  if (app->exposeSignals())
    flags_.set(BIT_SERVER_EVENT);

  ownerRepaint();
}

void EventSignalBase::ownerRepaint()
{
  flags_.set(BIT_NEED_UPDATE, true);
  owner()->signalConnectionsChanged();
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
	&& c.slot->type() == WStatelessSlot::SlotType::PreLearnStateless) {
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
	&& c.slot->type() == WStatelessSlot::SlotType::AutoLearnStateless) {
      learner->learn(c.slot);
      changed = true;
    }
  }

  if (changed)
    ownerRepaint();
}

}
