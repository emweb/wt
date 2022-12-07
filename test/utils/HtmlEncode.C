/*
 * Copyright (C) 2022 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <boost/test/unit_test.hpp>

#include <Wt/Utils.h>
#include <Wt/WWebWidget.h>

using Wt::Utils::htmlAttributeValue;

// Test if the example in the documentation compiles and works properly
BOOST_AUTO_TEST_CASE(htmlAttributeValue_test_doc)
{
  const std::string value = "Here's a value!";
  const std::string attribute = "name=\"" + htmlAttributeValue(value) + "\"";

  BOOST_REQUIRE_EQUAL("name=\"Here's a value!\"", attribute);
}

BOOST_AUTO_TEST_CASE(htmlAttributeValue_test_escape)
{
  const std::vector<std::pair<std::string, std::string>> tests = {
    {"", ""},
    {R"=(escape" <this! &>)=", R"=(escape&#34; &lt;this! &amp;>)="},
    {R"=(""&"<<&)=", R"=(&#34;&#34;&amp;&#34;&lt;&lt;&amp;)="},
    {"Nothing to escape", "Nothing to escape"},
  };

  for (const auto& test : tests) {
    const auto& value = test.first;
    const auto& result = test.second;

    BOOST_REQUIRE_EQUAL(result, htmlAttributeValue(value));

    // roundtrip test
    std::string toUnescape = htmlAttributeValue(value);
    BOOST_REQUIRE_EQUAL(value, Wt::WWebWidget::unescapeText(toUnescape));
  }
}
