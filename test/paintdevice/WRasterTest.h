// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2011 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WRASTER_TEST_H_
#define WRASTER_TEST_H_

#include <boost/test/unit_test.hpp>

using boost::unit_test_framework::test_suite;
using boost::unit_test_framework::test_case;

class WRasterTest : public test_suite
{
public:
  void test_dataUriImage();
  void test_textRenderer();

  WRasterTest();
};

#endif // WRASTER_TEST_H_
