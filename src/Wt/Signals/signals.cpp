/*
 * Copyright (C) 2016 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "signals.hpp"

namespace Wt { namespace Signals { namespace Impl {

SignalLinkBase::SignalLinkBase()
  : connected_(false),
    connection_ring_(nullptr)
{ }

SignalLinkBase::~SignalLinkBase()
{
  if (connection_ring_)
    connection_ring_->unlinkAll();
}

Connection SignalLinkBase::connect(const Wt::Core::observable *object)
{
  assert (!connected_);

  connected_ = true;
  obj_.reset(object);

  return Connection(this);
}

void SignalLinkBase::disconnect()
{
  connected_ = false;
}

bool SignalLinkBase::isConnected() const
{
  return connected_ && !obj_.observedDeleted();
}

Connection::Connection()
  : next_(nullptr),
    prev_(nullptr),
    signalLink_(nullptr)
{ }

Connection::Connection(const Connection& conn)
  : next_(nullptr),
    prev_(nullptr),
    signalLink_(nullptr)
{
  *this = conn;
}

Connection::~Connection()
{
  clear();
}

Connection& Connection::operator= (const Connection& other)
{
  clear();

  if (other.isConnected()) {
    signalLink_ = other.signalLink_;
    next_ = &other;
    prev_ = other.prev_;
    next_->prev_ = this;
    prev_->next_ = this;
  }

  return *this;
}

void Connection::disconnect()
{  
  if (signalLink_)
    signalLink_->disconnect();
}

bool Connection::isConnected() const
{
  if (signalLink_)
    return signalLink_->isConnected();
  else
    return false;
}

Connection::Connection(SignalLinkBase *signalLink)
  : next_(this),
    prev_(this),
    signalLink_(signalLink)
{
  if (signalLink_->connection_ring_) {
    next_ = signalLink_->connection_ring_;
    prev_ = signalLink_->connection_ring_->prev_;
    next_->prev_ = this;
    prev_->next_ = this;
  } else {
    signalLink_->connection_ring_ = this;
  }
}

void Connection::clear()
{
  if (next_) {
    bool lastConnection = next_ == this;

    if (signalLink_) {
      if (lastConnection)
	signalLink_->connection_ring_ = nullptr;
      else if (signalLink_->connection_ring_ == this)
	signalLink_->connection_ring_ = next_;
    }

    next_->prev_ = prev_;
    prev_->next_ = next_;
  }

  prev_ = next_ = nullptr;
  signalLink_ = nullptr;
}

void Connection::unlinkAll() const
{
  signalLink_ = nullptr;

  const Connection *c = next_;
  while (c && c != this) {
    c->signalLink_ = nullptr;
    c = c->next_;
  }
}

}}}
