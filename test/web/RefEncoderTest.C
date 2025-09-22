/*
 * Copyright (C) 2025 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WString.h"

#include "thirdparty/rapidxml/rapidxml.hpp"
#include "web/RefEncoder.h"

#include <boost/test/unit_test.hpp>

namespace {
  void performAllRefEncodeChecks(const std::string& content, const std::string& quotedContent = "")
  {
    const std::string compareTo = quotedContent.empty() ? content : quotedContent;
    Wt::WString result = Wt::EncodeRefs(content, {});

    BOOST_TEST(result == compareTo);

    result = Wt::EncodeRefs(content, { Wt::RefEncoderOption::EncodeInternalPaths });

    BOOST_TEST(result == compareTo);

    result = Wt::EncodeRefs(content, { Wt::RefEncoderOption::EncodeRedirectTrampoline });

    BOOST_TEST(result == compareTo);

    result = Wt::EncodeRefs(content, { Wt::RefEncoderOption::EncodeInternalPaths | Wt::RefEncoderOption::EncodeRedirectTrampoline });

    BOOST_TEST(result == compareTo);
  }
}

BOOST_AUTO_TEST_CASE( encode_plain_text )
{
  // Test showing that the refencoder will correctly parse a regular string.
  std::string content = "this is some simple content";
  performAllRefEncodeChecks(content);
}

BOOST_AUTO_TEST_CASE( encode_html_text )
{
  // Test showing that the refencoder will correctly parse a simple HTML block.
  std::string content = "<p>this is some html content</p>";
  performAllRefEncodeChecks(content);
}

BOOST_AUTO_TEST_CASE( encode_unbalanced_html_text )
{
  // Test showing that the refencoder will encounter a parse error with an unbalanced HTML block.
  std::string content = "<p>this is some html content";
  BOOST_CHECK_THROW(Wt::EncodeRefs(content, {}), Wt::rapidxml::parse_error);
}

BOOST_AUTO_TEST_CASE( encode_attributed_html_text )
{
  // Test showing that the refencoder will correctly parse a more complex HTML block.
  std::string content = "<p class=\"class\" style=\"width: 100%; color: red;\">this is some html content</p>";
  performAllRefEncodeChecks(content);
}

BOOST_AUTO_TEST_CASE( encode_attributed_quote_html_text )
{
  // Test showing that the refencoder will correctly parse a more complex HTML block, and replace a single quote.
  std::string content = "<p class='class'>this is some html content</p>";
  std::string quotedContent = "<p class=\"class\">this is some html content</p>";
  performAllRefEncodeChecks(content, quotedContent);
}

BOOST_AUTO_TEST_CASE( encode_href_attribute_no_params_html_text )
{
  // Test showing that the refencoder will correctly parse a more complex HTML block (with a simple href).
  std::string content = "<p href=\"www.emweb.be\" class=\"class\">this is some html content</p>";
  performAllRefEncodeChecks(content);
}

BOOST_AUTO_TEST_CASE( encode_href_attribute_with_params_html_text )
{
  // Test showing that the refencoder will encounter a parse error when the href isn't properly entity encoded.
  std::string content = "<p href=\"www.emweb.be?param1=1&param2=2\" class=\"class\">this is some html content</p>";
  BOOST_CHECK_THROW(Wt::EncodeRefs(content, {}), Wt::rapidxml::parse_error);
}

BOOST_AUTO_TEST_CASE( encode_href_attribute_with_params_and_regular_html_text )
{
  // Test showing that the refencoder will encounter a parse error when the href isn't properly entity encoded.
  // Should we fix #13996 (such that WTemplate is more context aware), this will change such that the ampersand
  // in the href becomes encoded to &amp;, and the last ampersand in the text will not be encoded.
  std::string content = "<p href=\"www.emweb.be?param1=1&param2=2\" class=\"class\">this is some html content, with an &</p>";
  BOOST_CHECK_THROW(Wt::EncodeRefs(content, {}), Wt::rapidxml::parse_error);
}
