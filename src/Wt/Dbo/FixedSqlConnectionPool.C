/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/Dbo/FixedSqlConnectionPool"
#include "Wt/Dbo/SqlConnection"

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
  boost::mutex::scoped_lock lock(mutex_);

  while (freeList_.empty())
    connectionAvailable_.wait(mutex_);

  SqlConnection *result = freeList_.back();
  freeList_.pop_back();

  return result;
}

void FixedSqlConnectionPool::returnConnection(SqlConnection *connection)
{
  boost::mutex::scoped_lock lock(mutex_);

  freeList_.push_back(connection);

  if (freeList_.size() == 1)
    connectionAvailable_.notify_one();
}

  }
}
