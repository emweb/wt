/*
 * Copyright (C) 2026 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include <Wt/Test/WTestEnvironment.h>

#include <Wt/WApplication.h>
#include <Wt/WString.h>
#include <Wt/WWebWidget.h>

#include <web/FileUtils.h>

namespace {
  struct XSSFixture
  {
  public:
    XSSFixture()
     : app_(environment_)
    {
      useBundle("web/xss");
    }

    void useBundle(const std::string &resourceName)
    {
      std::string file = app_.appRoot() + resourceName;
      if (!Wt::FileUtils::exists(file + ".xml")) {
        Wt::log("error") << "Test error: the file: '" + file + ".xml' is required to exist."
                        << " Are you running the test from the right directory?";
      }

      BOOST_REQUIRE(Wt::FileUtils::exists(file + ".xml"));
      app_.messageResourceBundle().use(file);
      BOOST_REQUIRE(Wt::WString::tr("check").toUTF8() == "Found");
    }

  private:
    Wt::Test::WTestEnvironment environment_;
    Wt::WApplication app_;
  };
}

BOOST_AUTO_TEST_CASE( test_XSSFilter_XSSFilterRemoveScript_literal_plain )
{
  // Tests whether a literal plain string correctly is kept as-is.

  Wt::WString content = "Hello, I am a string!";
  Wt::WString expected = "Hello, I am a string!";

  bool result = Wt::WWebWidget::removeScript(content);

  BOOST_REQUIRE(result);
  BOOST_CHECK_EQUAL(content, expected);
}

BOOST_AUTO_TEST_CASE( test_XSSFilter_XSSFilterRemoveScript_literal_xhtml )
{
  // Tests whether a literal XHTML string correctly is kept as-is.

  Wt::WString content = "<p>Hello, I am a string!</p>";
  Wt::WString expected = "<p>Hello, I am a string!</p>";

  bool result = Wt::WWebWidget::removeScript(content);

  BOOST_REQUIRE(result);
  BOOST_CHECK_EQUAL(content, expected);
}

BOOST_AUTO_TEST_CASE( test_XSSFilter_XSSFilterRemoveScript_literal_unsafe_xhtml_tag )
{
  // Tests whether a literal unsafe XHTML string correctly filters out
  // the bad `script` tag.

  Wt::WString content = "<p><script>var j = 5;</script>Hello, I am a string!</p>";
  Wt::WString expected = "<p>Hello, I am a string!</p>";

  bool result = Wt::WWebWidget::removeScript(content);

  BOOST_REQUIRE(result);
  BOOST_CHECK_EQUAL(content, expected);
}

BOOST_AUTO_TEST_CASE( test_XSSFilter_XSSFilterRemoveScript_literal_unsafe_xhtml_attribute )
{
  // Tests whether a literal unsafe XHTML string correctly filters out
  // the bad `onclick` attribute.

  Wt::WString content = "<p onclick='var j = 5;'>Hello, I am a string!</p>";
  Wt::WString expected = "<p>Hello, I am a string!</p>";

  bool result = Wt::WWebWidget::removeScript(content);

  BOOST_REQUIRE(result);
  BOOST_CHECK_EQUAL(content, expected);
}

BOOST_AUTO_TEST_CASE( test_XSSFilter_XSSFilterRemoveScript_literal_unsafe_xhtml_attribute_value )
{
  // Tests whether a literal unsafe XHTML string correctly filters out
  // the bad `javascript:` attribute value for the allowed `href`
  // attribute.

  Wt::WString content = "<a href='javascript:doSomething();'>Hello, I am a string!</a>";
  Wt::WString expected = "<a>Hello, I am a string!</a>";

  bool result = Wt::WWebWidget::removeScript(content);

  BOOST_REQUIRE(result);
  BOOST_CHECK_EQUAL(content, expected);
}

BOOST_AUTO_TEST_CASE( test_XSSFilter_XSSFilterRemoveScript_literal_xhtml_with_entities )
{
  // Tests whether a literal XHTML string correctly keeps any XHTML
  // entity in the string as-is.

  Wt::WString content = "<p>Hello &amp; I am a string!</p>";
  Wt::WString expected = "<p>Hello &amp; I am a string!</p>";

  bool result = Wt::WWebWidget::removeScript(content);

  BOOST_REQUIRE(result);
  BOOST_CHECK_EQUAL(content, expected);
}

BOOST_FIXTURE_TEST_CASE( test_XSSFilter_XSSFilterRemoveScript_resolved_plain, XSSFixture )
{
  // Tests whether a resolved plain string correctly is kept as-is.

  Wt::WString content = Wt::WString::tr("plain");
  Wt::WString expected = "Hello, I am a string!";

  bool result = Wt::WWebWidget::removeScript(content);

  BOOST_REQUIRE(result);
  BOOST_CHECK_EQUAL(content, expected);
}

BOOST_FIXTURE_TEST_CASE( test_XSSFilter_XSSFilterRemoveScript_resolved_xhtml, XSSFixture )
{
  // Tests whether a resolved XHTML string correctly is kept as-is.

  Wt::WString content = Wt::WString::tr("xhtml");
  Wt::WString expected = "<p>Hello, I am a string!</p>";

  bool result = Wt::WWebWidget::removeScript(content);

  BOOST_REQUIRE(result);
  BOOST_CHECK_EQUAL(content, expected);
}

BOOST_FIXTURE_TEST_CASE( test_XSSFilter_XSSFilterRemoveScript_resolved_unsafe_xhtml_tag, XSSFixture )
{
  // Tests whether a literal unsafe XHTML string correctly filters out
  // the bad `script` tag.

  Wt::WString content = Wt::WString::tr("unsafe-xhtml-script");
  Wt::WString expected = "<p>Hello, I am a string!</p>";

  bool result = Wt::WWebWidget::removeScript(content);

  BOOST_REQUIRE(result);
  BOOST_CHECK_EQUAL(content, expected);
}

BOOST_FIXTURE_TEST_CASE( test_XSSFilter_XSSFilterRemoveScript_resolved_unsafe_xhtml_attribute, XSSFixture )
{
  // Tests whether a literal unsafe XHTML string correctly filters out
  // the bad `onclick` attribute.

  Wt::WString content = Wt::WString::tr("unsafe-xhtml-attribute");
  Wt::WString expected = "<p>Hello, I am a string!</p>";

  bool result = Wt::WWebWidget::removeScript(content);

  BOOST_REQUIRE(result);
  BOOST_CHECK_EQUAL(content, expected);
}

BOOST_FIXTURE_TEST_CASE( test_XSSFilter_XSSFilterRemoveScript_resolved_unsafe_xhtml_attribute_value, XSSFixture )
{
  // Tests whether a literal unsafe XHTML string correctly filters out
  // the bad `javascript:` attribute value for the allowed `href`
  // attribute.

  Wt::WString content = Wt::WString::tr("unsafe-xhtml-attribute-value");
  Wt::WString expected = "<a>Hello, I am a string!</a>";

  bool result = Wt::WWebWidget::removeScript(content);

  BOOST_REQUIRE(result);
  BOOST_CHECK_EQUAL(content, expected);
}

BOOST_FIXTURE_TEST_CASE( test_XSSFilter_XSSFilterRemoveScript_resolved_xhtml_with_entities, XSSFixture )
{
  // Tests whether a literal XHTML string correctly keeps any XHTML
  // entity in the string as-is.

  Wt::WString content = Wt::WString::tr("xhtml-with-entity");
  Wt::WString expected = "<p>Hello &amp; I am a string!</p>";

  bool result = Wt::WWebWidget::removeScript(content);

  BOOST_REQUIRE(result);
  BOOST_CHECK_EQUAL(content, expected);
}
