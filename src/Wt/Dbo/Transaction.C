/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <iostream>

#include "Wt/Dbo/Transaction.h"
#include "Wt/Dbo/SqlConnection.h"
#include "Wt/Dbo/Session.h"
#include "Wt/Dbo/ptr.h"

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

/*
 * About noexcept(false), see
 * http://akrzemi1.wordpress.com/2011/09/21/destructors-that-throw/
 */
Transaction::~Transaction() noexcept(false)
{
  // Either this Transaction shell was not committed (first condition)
  // or the commit failed (we are still active and need to rollback)
  if (!committed_ || impl_->needsRollback_) {
    // A commit attempt failed (and thus we need to rollback) or we
    // are unwinding a stack while an exception is thrown
    if (impl_->needsRollback_ || std::uncaught_exception()) {
      bool canThrow = !std::uncaught_exception();
      try {
	rollback();
      } catch (...) {
	release();
	if (canThrow)
	  throw;
      }
    } else {
      try {
	commit();
      } catch (...) {
	try {
	  if (impl_->transactionCount_ == 1)
	    rollback();
	} catch (...) {
	  std::cerr << "Unexpected exception during Transaction::rollback()"
		    << std::endl;
	}

	release();
	throw;
      }
    }
  }

  release();
}

void Transaction::release()
{
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

Session& Transaction::session() const
{
  return session_;
}

SqlConnection *Transaction::connection() const
{
  impl_->open();
  return impl_->connection_.get();
}

Transaction::Impl::Impl(Session& session)
  : session_(session),
    active_(true),
    needsRollback_(false),
    open_(false),
    transactionCount_(0)
{
  connection_ = session_.useConnection();
}

Transaction::Impl::~Impl()
{
  if (connection_)
    session_.returnConnection(std::move(connection_));
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
  needsRollback_ = true;
  if (session_.flushMode() == FlushMode::Auto)
    session_.flush();

  if (open_)
    connection_->commitTransaction();

  for (unsigned i = 0; i < objects_.size(); ++i) {
    objects_[i]->transactionDone(true);
    delete objects_[i];
  }

  objects_.clear();

  session_.returnConnection(std::move(connection_));
  session_.transaction_ = nullptr;
  active_ = false;
  needsRollback_ = false;
}

void Transaction::Impl::rollback()
{
  needsRollback_ = false;

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

  objects_.clear();


  session_.returnConnection(std::move(connection_));
  session_.transaction_ = nullptr;
  active_ = false;
}

  }
}
