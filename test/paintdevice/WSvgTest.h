// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WSVG_TEST_H_
#define WSVG_TEST_H_

#include <boost/test/unit_test.hpp>

using boost::unit_test_framework::test_suite;
using boost::unit_test_framework::test_case;

class WSvgTest : public test_suite
{
public:
  void test_drawWrappedText();
  void test_drawSingleText();

  WSvgTest();
};

#endif // WSVG_TEST_H_
