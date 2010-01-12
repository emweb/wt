/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <iostream>

#include "Wt/Dbo/Transaction"
#include "Wt/Dbo/SqlConnection"
#include "Wt/Dbo/Session"
#include "Wt/Dbo/ptr"

namespace Wt {
  namespace Dbo {

Transaction::Transaction(Session& session)
  : committed_(false),
    session_(session)
{ 
  if (!session_.transaction_)
    session_.transaction_ = new Impl(session_);

  impl_ = session_.transaction_;

  ++impl_->transactionCount_;
}

Transaction::~Transaction()
{
  if (!committed_)
    rollback();

  --impl_->transactionCount_;

  if (impl_->transactionCount_ == 0)
    delete impl_;
}

bool Transaction::isActive() const
{
  return impl_->active_;
}

bool Transaction::commit()
{
  if (isActive()) {
    committed_ = true;

    if (impl_->transactionCount_ == 1) {
      impl_->commit();

      return true;
    } else
      return false;
  } else
    return false;
}

void Transaction::rollback()
{
  if (isActive())
    impl_->rollback();
}

Transaction::Impl::Impl(Session& session)
  : session_(session),
    active_(true),
    transactionCount_(0)
{ 
  connection_ = session_.useConnection();
  connection_->startTransaction();
}

void Transaction::Impl::commit()
{
  session_.flush();

  connection_->commitTransaction();
  session_.returnConnection(connection_);
  session_.transaction_ = 0;
  active_ = false;

  for (unsigned i = 0; i < objects_.size(); ++i) {
    objects_[i]->transactionDone(true);
    delete objects_[i];
  }

  objects_.clear();
}

void Transaction::Impl::rollback()
{
  connection_->rollbackTransaction();
  session_.returnConnection(connection_);
  session_.transaction_ = 0;
  active_ = false;

  for (unsigned i = 0; i < objects_.size(); ++i) {
    objects_[i]->transactionDone(false);
    delete objects_[i];
  }

  objects_.clear();
}

  }
}
