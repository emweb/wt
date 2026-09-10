// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2021 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef TEST_SESSION_H_
#define TEST_SESSION_H_

#include <Wt/Dbo/Session.h>

class TestSession : public Wt::Dbo::Session
{
public:
  TestSession(Wt::Dbo::SqlConnectionPool& pool, bool createTables);

  static std::unique_ptr<Wt::Dbo::SqlConnectionPool> createConnectionPool(const std::string& sqlite3);

private:
  void init(bool createTables);
  void initTables();
};

#endif // TEST_SESSION_H_
