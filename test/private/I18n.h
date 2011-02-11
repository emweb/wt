// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2011 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifndef C_I18N_H
#define C_I18N_H

#include <boost/test/unit_test.hpp>

using boost::unit_test_framework::test_suite;
using boost::unit_test_framework::test_case;

class I18n : public test_suite
{
public:
  I18n();

private:
  void setup();
  void teardown();

  std::string trn(const std::string &key, int n);

  void messageResourceBundleTest();

  void pluralResourceBundleException(const std::string &resourceName);
  void pluralResourceBundleException1();
  void pluralResourceBundleException2();
  void pluralResourceBundleException3();
  void pluralResourceBundleException4();
  void pluralResourceBundleException5();

  void pluralResourceBundle1();

  void findCaseException1();
  void findCaseException2();
};

#endif // C_I18N_H
