/*
 * Copyright (C) 2022 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include <Wt/Test/WTestEnvironment.h>
#include <Wt/WApplication.h>
#include <Wt/WLocalizedStrings.h>
#include <Wt/WText.h>

class LocalizedStrings final : public Wt::WLocalizedStrings {
public:
  Wt::LocalizedString resolveKey(const Wt::WLocale &locale, const std::string &key) override
  {
    if (key == "lang") {
      if (locale.name() == "nl") {
        return {"Nederlands {1}", Wt::TextFormat::XHTML};
      } else {
        return {"English {1}", Wt::TextFormat::XHTML};
      }
    } else {
      return {};
    }
  }
};

BOOST_AUTO_TEST_CASE(WText_test_xhtml_malicious)
{
  Wt::Test::WTestEnvironment testEnv;
  Wt::WApplication app(testEnv);

  Wt::WText text;
  text.setText(Wt::WString("<script>alert('boo');</script>"));
  BOOST_TEST(text.text().toUTF8() == "");
}

BOOST_AUTO_TEST_CASE(WText_test_unsafexhtml)
{
  Wt::Test::WTestEnvironment testEnv;
  Wt::WApplication app(testEnv);

  Wt::WText text;
  text.setTextFormat(Wt::TextFormat::UnsafeXHTML);
  text.setText(Wt::WString("<script>alert('boo');</script>"));
  BOOST_TEST(text.text().toUTF8() == "<script>alert('boo');</script>");
}

BOOST_AUTO_TEST_CASE(WText_test_xhtml_nonliteral)
{
  Wt::Test::WTestEnvironment env;
  Wt::WApplication app(env);
  app.setLocalizedStrings(std::make_shared<LocalizedStrings>());
  app.setLocale(Wt::WLocale("en"));

  Wt::WText text;
  text.setText(Wt::WString::tr("lang").arg(1));
  BOOST_TEST(text.text().toUTF8() == "English 1");

  app.setLocale(Wt::WLocale("nl"));
  BOOST_TEST(text.text().toUTF8() == "Nederlands 1");
}

