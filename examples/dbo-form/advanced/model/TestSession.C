/*
 * Copyright (C) 2021 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "TestSession.h"

#include "TestDboObject.h"

#include <Wt/Dbo/FixedSqlConnectionPool.h>
#include <Wt/Dbo/SqlConnection.h>
#include <Wt/Dbo/backend/Sqlite3.h>

std::unique_ptr<Wt::Dbo::SqlConnectionPool> TestSession::createConnectionPool(const std::string& sqliteDb)
{
  auto connection = std::make_unique<Wt::Dbo::backend::Sqlite3>(sqliteDb);

  return std::make_unique<Wt::Dbo::FixedSqlConnectionPool>(std::move(connection), 10);
}

TestSession::TestSession(Wt::Dbo::SqlConnectionPool& pool, bool createTables)
{
  setConnectionPool(pool);

  init(createTables);
}

void TestSession::init(bool createTables)
{
  mapClass<TestDboPtr>("test_dbo_ptr");
  mapClass<TestDboObject>("test_dbo_object");

  if (createTables) {
    try {
      this->createTables();
      Wt::log("info") << "Created database.";
      this->initTables();
      Wt::log("info") << "Initialized tables.";
    } catch (std::exception& e) {
      Wt::log("info") << e.what();
      Wt::log("info") << "Using existing database.";
    }
  }
}

void TestSession::initTables()
{
  Wt::Dbo::Transaction t(*this);

  addNew<TestDboPtr>("Ptr 1");
  addNew<TestDboPtr>("Ptr 2");
  addNew<TestDboPtr>("Ptr 3");
  addNew<TestDboPtr>("Ptr 4");
  addNew<TestDboPtr>("Ptr 5");

  addNew<TestDboObject>();
}
