/*
 * Copyright (C) 2016 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "signals.hpp"

namespace Wt { namespace Signals { namespace Impl {

SignalLinkBase::SignalLinkBase(CBUnlink unlink_callback)
  : connected_(false),
    connection_ring_(nullptr),
    unlink_callback_(unlink_callback)
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
  if (connection_ring_)
    connection_ring_->unlinkAll();
  connection_ring_ = nullptr;
  connected_ = false;
  unlink_callback_(this);
}

bool SignalLinkBase::isConnected() const
{
  return connected_ && !obj_.observedDeleted();
}

Connection::Connection() noexcept
  : next_(nullptr),
    prev_(nullptr),
    signalLink_(nullptr)
{ }

Connection::Connection(const Connection& conn) noexcept
  : next_(nullptr),
    prev_(nullptr),
    signalLink_(nullptr)
{
  *this = conn;
}

Connection::Connection(Connection&& conn) noexcept
  : next_(nullptr),
    prev_(nullptr),
    signalLink_(nullptr)
{
  *this = std::move(conn);
}

Connection::~Connection() noexcept
{
  clear();
}

Connection& Connection::operator= (const Connection& other) noexcept
{
  if (this == &other)
    return *this;

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

Connection& Connection::operator= (Connection&& other) noexcept
{
  if (this == &other)
    return *this;

  const Connection &cother = other;
  operator=(cother);
  other.clear();

  return *this;
}

void Connection::disconnect() noexcept
{  
  if (signalLink_)
    signalLink_->disconnect();
}

bool Connection::isConnected() const noexcept
{
  if (signalLink_)
    return signalLink_->isConnected();
  else
    return false;
}

Connection::Connection(SignalLinkBase *signalLink) noexcept
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

void Connection::clear() noexcept
{
  if (next_) {
    bool lastConnection = next_ == this;
    assert(lastConnection || prev_ != this);

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

void Connection::unlinkAll() const noexcept
{
  signalLink_ = nullptr;

  const Connection *c = next_;
  while (c && c != this) {
    c->signalLink_ = nullptr;
    auto cnext = c->next_;
    c->next_ = nullptr;
    c->prev_ = nullptr;
    c = cnext;
  }

  next_ = nullptr;
  prev_ = nullptr;
}

}}}
