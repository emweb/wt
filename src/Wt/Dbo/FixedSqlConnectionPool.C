/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/Dbo/FixedSqlConnectionPool"
#include "Wt/Dbo/SqlConnection"
#include "Wt/Dbo/Exception"

namespace Wt {
  namespace Dbo {

FixedSqlConnectionPool::FixedSqlConnectionPool(SqlConnection *connection,
					       int size)
{
  freeList_.push_back(connection);

  for (int i = 1; i < size; ++i)
    freeList_.push_back(connection->clone());
}

FixedSqlConnectionPool::~FixedSqlConnectionPool()
{
  for (unsigned i = 0; i < freeList_.size(); ++i)
    delete freeList_[i];
}

SqlConnection *FixedSqlConnectionPool::getConnection()
{
#ifdef WT_THREADED
  boost::mutex::scoped_lock lock(mutex_);

  while (freeList_.empty())
    connectionAvailable_.wait(mutex_);
#else
  if (freeList_.empty())
    throw Exception("FixedSqlConnectionPool::getConnection(): "
		    "no connection available but single-threaded build?");
#endif // WT_THREADED

  SqlConnection *result = freeList_.back();
  freeList_.pop_back();

  return result;
}

void FixedSqlConnectionPool::returnConnection(SqlConnection *connection)
{
#ifdef WT_THREADED
  boost::mutex::scoped_lock lock(mutex_);
#endif // WT_THREADED

  freeList_.push_back(connection);

#ifdef WT_THREADED
  if (freeList_.size() == 1)
    connectionAvailable_.notify_one();
#endif // WT_THREADED
}

  }
}
