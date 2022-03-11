/*
 * Copyright (C) 2022 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include "web/WebUtils.h"

#include "3rdparty/rapidxml/rapidxml.hpp"
#include "3rdparty/rapidxml/rapidxml_print.hpp"

#include <vector>

BOOST_AUTO_TEST_CASE(fixSelfClosingTags_test1)
{
  std::string input = R"(<script src="myscript.js"></script>)";
  std::vector<char> text(input.begin(), input.end());
  text.push_back('\0');

  Wt::rapidxml::xml_document<> doc;
  doc.parse<0>(&text[0]);
  Wt::Utils::fixSelfClosingTags(&doc);

  std::string output;
  Wt::rapidxml::print(std::back_inserter(output), doc,
                      Wt::rapidxml::print_no_indenting);
  BOOST_CHECK_EQUAL(output, input);
}
