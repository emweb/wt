// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/Dbo/ptr.h>
#include <Wt/Dbo/Exception.h>
#include <Wt/Dbo/Session.h>

namespace Wt {
  namespace Dbo {

MetaDboBase::~MetaDboBase()
{ }

void MetaDboBase::transactionDone(bool success)
{
  doTransactionDone(success);
}

void MetaDboBase::incRef()
{
  ++refCount_;
}

void MetaDboBase::decRef()
{
  --refCount_;

  if (refCount_ == 0)
    delete this;
}

void MetaDboBase::setState(State state)
{
  state_ &= ~0xF;
  state_ |= state;
}

void MetaDboBase::setDirty()
{
  checkNotOrphaned();
  if (isDeleted()) {
    return;
  }

  if (!isDirty()) {
    state_ |= NeedsSave;
    if (session_)
      session_->needsFlush(this);
  }
}

void MetaDboBase::remove()
{
  checkNotOrphaned();

  if (isDeleted()) {
    // is already removed or being removed in this transaction
  } else if (isPersisted()) {
    state_ |= NeedsDelete;
    session_->needsFlush(this);
  } else if (session_) { // was added to a Session but not yet flushed
    Session *session = session_;
    setSession(nullptr);
    session->discardChanges(this);
    state_ &= ~NeedsSave;
  } else {
    // is not yet added to the Session
  }
}

void MetaDboBase::setTransactionState(State state)
{
  state_ &= ~Saving;
  state_ |= state;
}

void MetaDboBase::resetTransactionState()
{
  state_ &= ~TransactionState;
}

void MetaDboBase::checkNotOrphaned()
{
  if (isOrphaned()) {
    throw Exception("using orphaned dbo ptr");
  }
}

ptr_base::~ptr_base()
{ }

  }
}
