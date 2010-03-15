// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2009 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef DBO_TEST2_H
#define DBO_TEST2_H

#include <boost/test/unit_test.hpp>

namespace Wt {
  namespace Dbo {
    class SqlConnection;
    class Session;
  }
}

using boost::unit_test_framework::test_suite;
using boost::unit_test_framework::test_case;

class DboTest2 : public test_suite
{
private:
  Wt::Dbo::SqlConnection *connection_;
  Wt::Dbo::Session *session_;

  void setup();
  void teardown();

public:
  void test1();

  DboTest2();
};

#endif // DBO_TEST2_H
