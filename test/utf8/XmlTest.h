// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2009 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef XML_TEST_H_
#define XML_TEST_H_

#include <boost/test/unit_test.hpp>

using boost::unit_test_framework::test_suite;
using boost::unit_test_framework::test_case;

class XmlTest : public test_suite
{
public:
  void test();

  XmlTest();
};

#endif // XML_TEST_H_
