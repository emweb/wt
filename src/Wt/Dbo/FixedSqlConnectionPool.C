/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/Dbo/FixedSqlConnectionPool"
#include "Wt/Dbo/SqlConnection"
#include "Wt/Dbo/Exception"

#ifdef WT_THREADED
#include <boost/thread.hpp>
#include <boost/thread/condition.hpp>
#else
#endif // WT_THREADED

#include <iostream>

namespace Wt {
  namespace Dbo {

struct FixedSqlConnectionPool::Impl {
#ifdef WT_THREADED
  boost::mutex mutex;
  boost::condition connectionAvailable;
#endif // WT_THREADED

  int timeout;
  std::vector<SqlConnection *> freeList;

  Impl()
    : timeout(0)
  { }
};

FixedSqlConnectionPool::FixedSqlConnectionPool(SqlConnection *connection,
					       int size)
{
  impl_ = new Impl();
  
  impl_->freeList.push_back(connection);

  for (int i = 1; i < size; ++i)
    impl_->freeList.push_back(connection->clone());
}

FixedSqlConnectionPool::~FixedSqlConnectionPool()
{
  for (unsigned i = 0; i < impl_->freeList.size(); ++i)
    delete impl_->freeList[i];

  delete impl_;
}

void FixedSqlConnectionPool::setTimeout(int timeout)
{
  impl_->timeout = timeout;
}

int FixedSqlConnectionPool::timeout() const
{
  return impl_->timeout;
}
  
SqlConnection *FixedSqlConnectionPool::getConnection()
{
#ifdef WT_THREADED
  boost::mutex::scoped_lock lock(impl_->mutex);

  boost::system_time timeout
    = boost::get_system_time() + boost::posix_time::milliseconds(impl_->timeout);

  while (impl_->freeList.empty()) {
    std::cerr << "Warning: FixedSqlConnectionPool: waiting for connection" << std::endl;
    if (impl_->timeout > 0) {
      if (!impl_->connectionAvailable.timed_wait(impl_->mutex, timeout)) {
	handleTimeout();
	timeout = boost::get_system_time() + boost::posix_time::milliseconds(impl_->timeout);
      }
    } else
      impl_->connectionAvailable.wait(impl_->mutex);
  }
#else
  if (impl_->freeList.empty())
    throw Exception("FixedSqlConnectionPool::getConnection(): "
		    "no connection available but single-threaded build?");
#endif // WT_THREADED

  SqlConnection *result = impl_->freeList.back();
  impl_->freeList.pop_back();

  return result;
}

void FixedSqlConnectionPool::handleTimeout()
{
  throw Exception("FixedSqlConnectionPool::getConnection(): timeout");
}
  
void FixedSqlConnectionPool::returnConnection(SqlConnection *connection)
{
#ifdef WT_THREADED
  boost::mutex::scoped_lock lock(impl_->mutex);
#endif // WT_THREADED

  impl_->freeList.push_back(connection);

#ifdef WT_THREADED
  if (impl_->freeList.size() == 1)
    impl_->connectionAvailable.notify_one();
#endif // WT_THREADED
}

void FixedSqlConnectionPool::prepareForDropTables() const
{
  for (unsigned i = 0; i < impl_->freeList.size(); ++i)
    impl_->freeList[i]->prepareForDropTables();
}

  }
}
