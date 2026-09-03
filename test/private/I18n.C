/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include "Wt/Test/WTestEnvironment.h"
#include "Wt/WApplication.h"
#include "Wt/WMessageResourceBundle.h"
#include "Wt/WString.h"
#include "Wt/cpp17/filesystem.hpp"

#include "web/FileUtils.h"

#include <boost/algorithm/string/predicate.hpp>

#include <fstream>
#include <iostream>
#include <stdexcept>

namespace {

void pluralResourceBundleException(const std::string &resourceName)
{
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

  std::string file = app.appRoot() + resourceName;
  BOOST_REQUIRE(Wt::FileUtils::exists(file + ".xml"));
  
  app.messageResourceBundle().use(file);

  BOOST_REQUIRE(Wt::WString::tr("file").toUTF8() == "??file??");
}

std::string trn(const std::string &key, int n)
{
  return Wt::WString::trn(key, n).arg(n).toUTF8();
}

class TemporaryMessageResourceBundle
{
public:
  TemporaryMessageResourceBundle()
    : path_(Wt::FileUtils::createTempFileName())
  {
    if (path_.empty()) {
      throw std::runtime_error("Could not create a temporary file");
    }
  }

  ~TemporaryMessageResourceBundle()
  {
    Wt::cpp17::fs_error_code error;
    Wt::cpp17::filesystem::remove(path_, error);
    Wt::cpp17::filesystem::remove(fileName(""), error);
    Wt::cpp17::filesystem::remove(fileName("_nl"), error);
  }

  const std::string& path() const
  {
    return path_;
  }

  std::string fileName(const std::string& suffix) const
  {
    return path_ + suffix + ".xml";
  }

  void write(const std::string& suffix, const std::string& key,
             const std::string& value)
  {
    std::ofstream file(fileName(suffix).c_str(),
                       std::ios::binary | std::ios::trunc);
    file << "<messages><message id=\"" << key << "\">" << value
         << "</message></messages>";
    file.close();

    if (!file) {
      throw std::runtime_error("Could not write a temporary resource file");
    }
  }

private:
  std::string path_;
};

}

BOOST_AUTO_TEST_CASE( I18n_messageResourceBundleServerCache )
{
  TemporaryMessageResourceBundle files;
  files.write("", "fallback", "base");
  files.write("_nl", "localized", "initial");

  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);
  const Wt::WLocale locale("nl-BE");

  Wt::WMessageResourceBundle original;
  original.use(files.path(), false);
  BOOST_REQUIRE_EQUAL(original.resolveKey(locale, "localized").value,
                      "initial");
  BOOST_REQUIRE_EQUAL(original.resolveKey(locale, "fallback").value, "base");

  const std::string localizedFile = files.fileName("_nl");
  const auto originalWriteTime
    = Wt::cpp17::filesystem::last_write_time(localizedFile);
  const auto originalSize = Wt::cpp17::filesystem::file_size(localizedFile);

  files.write("_nl", "localized", "changed");
  Wt::cpp17::filesystem::last_write_time(localizedFile, originalWriteTime);
  BOOST_REQUIRE(Wt::cpp17::filesystem::last_write_time(localizedFile)
                == originalWriteTime);
  BOOST_REQUIRE_EQUAL(Wt::cpp17::filesystem::file_size(localizedFile),
                      originalSize);

  Wt::WMessageResourceBundle sameMetadata;
  sameMetadata.use(files.path());
  BOOST_REQUIRE_EQUAL(sameMetadata.resolveKey(locale, "localized").value,
                      "initial");
  BOOST_REQUIRE_EQUAL(sameMetadata.resolveKey(locale, "fallback").value,
                      "base");

  files.write("_nl", "localized", "refreshed");
  BOOST_REQUIRE(Wt::cpp17::filesystem::file_size(localizedFile)
                != originalSize);

  Wt::WMessageResourceBundle refreshed;
  refreshed.use(files.path());
  BOOST_REQUIRE_EQUAL(refreshed.resolveKey(locale, "localized").value,
                      "refreshed");
  BOOST_REQUIRE_EQUAL(refreshed.resolveKey(locale, "fallback").value, "base");

  const std::set<std::string> originalKeys = original.keys(locale);
  BOOST_REQUIRE(originalKeys.find("localized") != originalKeys.end());
  BOOST_REQUIRE_EQUAL(original.resolveKey(locale, "localized").value,
                      "initial");
  BOOST_REQUIRE_EQUAL(sameMetadata.resolveKey(locale, "localized").value,
                      "initial");

  sameMetadata.hibernate();
  BOOST_REQUIRE_EQUAL(sameMetadata.resolveKey(locale, "localized").value,
                      "initial");

  original.hibernate();
  BOOST_REQUIRE_EQUAL(original.resolveKey(locale, "localized").value,
                      "refreshed");
  BOOST_REQUIRE_EQUAL(original.resolveKey(locale, "fallback").value, "base");
}

BOOST_AUTO_TEST_CASE( I18n_messageResourceBundleTest )
{
  BOOST_REQUIRE(Wt::WString::tr("welcome-text").toUTF8() == "??welcome-text??");
  
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

  BOOST_REQUIRE(Wt::WString::tr("welcome-text").toUTF8() == "??welcome-text??");

  app.messageResourceBundle().use(app.appRoot() + "private/i18n/plain");

  std::string welcome = Wt::WString::tr("welcome-text").arg("Joske").toUTF8();
  BOOST_REQUIRE(welcome == 
                "Welcome dear visitor, Joske of the WFooBar magic website !");

  BOOST_REQUIRE(Wt::WString::tr("welcome").toUTF8() == "??welcome??");

  app.setLocale("nl");
  welcome = Wt::WString::tr("welcome-text").arg("Joske").toUTF8();
  BOOST_REQUIRE(welcome == 
                "Welkom beste bezoeker, Joske van de WFooBar magic website !");

  std::string programmer = Wt::WString::tr("programmer").toUTF8();
  BOOST_REQUIRE(programmer == "Programmer");

  app.setLocale("pl");
  welcome = Wt::WString::tr("welcome-text").arg("Joske").toUTF8();
  BOOST_REQUIRE(welcome == 
                "Welcome dear visitor, Joske of the WFooBar magic website !");
}

BOOST_AUTO_TEST_CASE( I18n_pluralResourceBundleException1 )
{
  pluralResourceBundleException("private/i18n/plural_err_1");
}

BOOST_AUTO_TEST_CASE( I18n_pluralResourceBundleException2 )
{
  pluralResourceBundleException("private/i18n/plural_err_2");
}

BOOST_AUTO_TEST_CASE( I18n_pluralResourceBundleException3 )
{
  pluralResourceBundleException("private/i18n/plural_err_3");
}

BOOST_AUTO_TEST_CASE( I18n_pluralResourceBundleException4 )
{
  pluralResourceBundleException("private/i18n/plural_err_4");
}

BOOST_AUTO_TEST_CASE( I18n_pluralResourceBundleException5 )
{
  pluralResourceBundleException("private/i18n/plural_err_5");
}

BOOST_AUTO_TEST_CASE( I18n_pluralResourceBundle1 )
{
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

  app.messageResourceBundle().use(app.appRoot() + "private/i18n/plural");

  BOOST_REQUIRE(trn("file", 0) == "0 files");
  BOOST_REQUIRE(trn("file", 1) == "1 file");
  BOOST_REQUIRE(trn("file", 6) == "6 files");

  BOOST_REQUIRE(Wt::WString::tr("welcome").toUTF8() == "??welcome??");

  BOOST_REQUIRE(trn("programmer", 0) == "0 programmers");
  BOOST_REQUIRE(trn("programmer", 1) == "1 programmer");
  BOOST_REQUIRE(trn("programmer", 6) == "6 programmers");

  app.setLocale("pl");
  
  BOOST_REQUIRE(trn("file", 0) == "0 pliko'w");
  BOOST_REQUIRE(trn("file", 1) == "1 plik");
  BOOST_REQUIRE(trn("file", 2) == "2 pliki");
  BOOST_REQUIRE(trn("file", 6) == "6 pliko'w");
  BOOST_REQUIRE(trn("file", 23) == "23 pliki");
  BOOST_REQUIRE(trn("file", 26) == "26 pliko'w");

  BOOST_REQUIRE(Wt::WString::tr("welcome").toUTF8() == "??welcome??");

  BOOST_REQUIRE(trn("programmer", 0) == "0 programmers");
  BOOST_REQUIRE(trn("programmer", 1) == "1 programmer");
  BOOST_REQUIRE(trn("programmer", 6) == "6 programmers");

  app.setLocale("nl");

  BOOST_REQUIRE(trn("file", 0) == "0 files");
  BOOST_REQUIRE(trn("file", 1) == "1 file");
  BOOST_REQUIRE(trn("file", 6) == "6 files");

  BOOST_REQUIRE(Wt::WString::tr("welcome").toUTF8() == "??welcome??");

  BOOST_REQUIRE(trn("programmer", 0) == "0 programmers");
  BOOST_REQUIRE(trn("programmer", 1) == "1 programmer");
  BOOST_REQUIRE(trn("programmer", 6) == "6 programmers");
}

BOOST_AUTO_TEST_CASE( I18n_findCaseException1 )
{
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

  app.messageResourceBundle().use("private/i18n/plural_findcase_err1");

  std::string error;
  try {
    Wt::WString::trn("file", 1).toUTF8();
  } catch (std::exception& e) {
    error = e.what();
  }

  BOOST_REQUIRE(boost::starts_with(error,
                "Expression 'n-10' evaluates to '-9' for n=1 and values "
                "smaller than 0 are not allowed."));
}

BOOST_AUTO_TEST_CASE( I18n_findCaseException2 )
{
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

  app.messageResourceBundle().use(app.appRoot() + 
                                  "private/i18n/plural_findcase_err2");

  std::string error;
  try {
    Wt::WString::trn("file", 1).toUTF8();
  } catch (std::exception& e) {
    error = e.what();
  }

  BOOST_REQUIRE(boost::starts_with(error,
                "Expression '2' evaluates to '2' for n=1 which is greater "
                "than the list of cases (size=1)."));
}

BOOST_AUTO_TEST_CASE( I18n_internalArgument1 )
{
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);
  
  app.messageResourceBundle().use(app.appRoot() + 
                                  "private/i18n/international_argument_1");
  
  Wt::WString text = Wt::WString::tr("text").arg(Wt::WString::tr("hello"));

  BOOST_REQUIRE(text.toUTF8() == 
                "Internationalized text containing an "
                "internationalized argument: hello");

  app.setLocale("nl");

  BOOST_REQUIRE(text.toUTF8() == 
                "Geïnternationaliseerde tekst met een geïnternationaliseerd "
                "argument: hallo");
}

BOOST_AUTO_TEST_CASE( I18n_badUTF8 )
{
  // "máquina quente do forró"
  const char badutf8[] = {'m', (char)225, 'q', 'u', 'i', 'n', 'a', ' ',
    'q', 'u', 'e', 'n', 't', 'e', ' ', 'd', 'o', ' ',
    'f', 'o', 'r', 'r', (char)243, 0};
  std::string badUTF8(badutf8);
  Wt::WString::checkUTF8Encoding(badUTF8);
}

BOOST_AUTO_TEST_CASE( I18n_toXhtmlUTF8 )
{
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

  app.messageResourceBundle().use(app.appRoot() +
                                  "private/i18n/toxhtml");

  Wt::WString text = Wt::WString::tr("support-training").arg(Wt::utf8("<a href=\"http://webtoolkit.eu\">Wt!</a>"));

  BOOST_REQUIRE(text.toXhtmlUTF8() == "Support &amp; Training <a href=\"http://webtoolkit.eu\">Wt!</a>");

  BOOST_REQUIRE(text.toUTF8() == "Support & Training <a href=\"http://webtoolkit.eu\">Wt!</a>");
}
