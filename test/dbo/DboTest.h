// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2009 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifndef DBO_TEST_H
#define DBO_TEST_H

#include <boost/test/unit_test.hpp>

namespace Wt {
  namespace Dbo {
    class SqlConnectionPool;
    class Session;
  }
}

using boost::unit_test_framework::test_suite;
using boost::unit_test_framework::test_case;

class DboTest : public test_suite
{
public:
  DboTest();

private:
  Wt::Dbo::SqlConnectionPool *connectionPool_;
  Wt::Dbo::Session *session_;

  void setup();
  void teardown();

  void testQueryParser();
  void test1();
  void test2();
  void test3();
  void test4();
  void test5();
  void test6();
  void test7();
  void test8();
  void test9();
  void test10();
  void test11();
  void test12();
  void test13();
};

#endif // DBO_TEST_H
