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
  --impl_->transactionCount_;

  // Either this Transaction shell was not committed, or the commit failed.
  if (!committed_ || (impl_->transactionCount_ == 0 && isActive())) {
    try {
      rollback();
      if (impl_->transactionCount_ == 0)
	delete impl_;
    } catch (std::exception& e) {
      if (impl_->transactionCount_ == 0)
	delete impl_;
      throw e;
    }
  } else if (impl_->transactionCount_ == 0)
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
    open_(false),
    transactionCount_(0)
{ 
  connection_ = session_.useConnection();
}

void Transaction::Impl::open()
{
  if (!open_) {
    open_ = true;
    connection_->startTransaction();
  }
}

void Transaction::Impl::commit()
{
  session_.flush();

  for (unsigned i = 0; i < objects_.size(); ++i) {
    objects_[i]->transactionDone(true);
    delete objects_[i];
  }

  if (open_)
    connection_->commitTransaction();

  session_.returnConnection(connection_);
  session_.transaction_ = 0;
  active_ = false;

  objects_.clear();
}

void Transaction::Impl::rollback()
{
  try {
    if (open_)
      connection_->rollbackTransaction();
  } catch (const std::exception& e) {
    std::cerr << "Transaction::rollback(): " << e.what() << std::endl;
  }

  for (unsigned i = 0; i < objects_.size(); ++i) {
    objects_[i]->transactionDone(false);
    delete objects_[i];
  }

  session_.returnConnection(connection_);
  session_.transaction_ = 0;
  active_ = false;

  objects_.clear();
}

  }
}
