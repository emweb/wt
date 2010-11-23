// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2009 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef UTF8_TEST_H_
#define UTF8_TEST_H_

#include <boost/test/unit_test.hpp>

using boost::unit_test_framework::test_suite;
using boost::unit_test_framework::test_case;

class Utf8Test : public test_suite
{
public:
  void test();
  void test2();
  void test3();
  void test4();

  Utf8Test();
};

#endif // UTF8_TEST_H_
