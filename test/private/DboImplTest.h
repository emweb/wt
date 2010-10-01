// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2009 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifndef DBO_IMPL_TEST_H
#define DBO_IMPL_TEST_H

#include <boost/test/unit_test.hpp>

using boost::unit_test_framework::test_suite;
using boost::unit_test_framework::test_case;

class DboImplTest : public test_suite
{
public:
  DboImplTest();

private:
  void test1();

  void parseSql(const std::string& sql,
		int listsCount,
		int fieldsCount,
		bool simpleSelect);
};

#endif // DBO_IMPL_TEST_H
