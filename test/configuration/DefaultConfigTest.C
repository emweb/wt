/*
 * Copyright (C) 2023 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include <web/Configuration.h>
#include <web/FileUtils.h>

#include <string>

using namespace std::string_literals;

BOOST_AUTO_TEST_CASE( test_default_config_file )
{
  BOOST_REQUIRE(Wt::FileUtils::exists(WT_TEST_CONFIG_XML));

  Wt::Configuration configuration("", "", WT_TEST_CONFIG_XML, nullptr);
}