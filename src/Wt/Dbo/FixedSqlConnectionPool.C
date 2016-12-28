/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/Dbo/FixedSqlConnectionPool.h"
#include "Wt/Dbo/SqlConnection.h"

#ifdef WT_THREADED
#include <thread>
#include <mutex>
#include <condition_variable>
#else
#include "Wt/Dbo/Exception.h"
#endif // WT_THREADED

#include <memory>

namespace Wt {
  namespace Dbo {

struct FixedSqlConnectionPool::Impl {
#ifdef WT_THREADED
  std::mutex mutex;
  std::condition_variable connectionAvailable;
#endif // WT_THREADED

  std::vector<std::unique_ptr<SqlConnection>> freeList;
};

FixedSqlConnectionPool::FixedSqlConnectionPool(std::unique_ptr<SqlConnection> connection,
					       int size)
  : impl_(new Impl)
{
  SqlConnection *conn = connection.get();
  impl_->freeList.push_back(std::move(connection));

  for (int i = 1; i < size; ++i)
    impl_->freeList.push_back(conn->clone());
}

FixedSqlConnectionPool::~FixedSqlConnectionPool()
{
  impl_->freeList.clear();
}

std::unique_ptr<SqlConnection> FixedSqlConnectionPool::getConnection()
{
#ifdef WT_THREADED
  std::unique_lock<std::mutex> lock(impl_->mutex);

  while (impl_->freeList.empty())
    impl_->connectionAvailable.wait(lock);
#else
  if (impl_->freeList.empty())
    throw Exception("FixedSqlConnectionPool::getConnection(): "
		    "no connection available but single-threaded build?");
#endif // WT_THREADED

  std::unique_ptr<SqlConnection> result = std::move(impl_->freeList.back());
  impl_->freeList.pop_back();

  return result;
}

void FixedSqlConnectionPool::returnConnection(std::unique_ptr<SqlConnection> connection)
{
#ifdef WT_THREADED
  std::unique_lock<std::mutex> lock(impl_->mutex);
#endif // WT_THREADED

  impl_->freeList.push_back(std::move(connection));

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
