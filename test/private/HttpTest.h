// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifndef HTTP_TEST_H
#define HTTP_TEST_H

#include <boost/test/unit_test.hpp>

using boost::unit_test_framework::test_suite;
using boost::unit_test_framework::test_case;

class HttpTest : public test_suite
{
public:
  HttpTest();

private:
  void setup();
  void teardown();

  void rangeTest1();
  void rangeTest2();
};

#endif // HTTP_TEST_H
