/*
 * Copyright (C) 2012 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include "web/WebUtils.h"

BOOST_AUTO_TEST_CASE( EraseWord_test1 )
{
  std::string line = "panel panel-default panel-folding panel-menu leaf fold";

  BOOST_REQUIRE(Wt::Utils::eraseWord(line, "panel-folding") == "panel panel-default panel-menu leaf fold");
  BOOST_REQUIRE(Wt::Utils::eraseWord(line, "fold") == "panel panel-default panel-folding panel-menu leaf");
  BOOST_REQUIRE(Wt::Utils::eraseWord(line, "panel") == "panel-default panel-folding panel-menu leaf fold");
}
