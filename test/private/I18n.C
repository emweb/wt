/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include "Wt/Test/WTestEnvironment.h"
#include "Wt/WApplication.h"
#include "Wt/WMessageResourceBundle.h"
#include "Wt/WServer.h"
#include "Wt/WString.h"
#include "Wt/cpp17/filesystem.hpp"

#include "web/FileUtils.h"

#include <boost/algorithm/string/predicate.hpp>

#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>
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
    : path_(Wt::FileUtils::createTempFileName()),
      revision_(0)
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
    Wt::cpp17::filesystem::remove(fileName("_nl-BE"), error);
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

    // Give each write a distinct timestamp, even on coarse filesystems.
    const auto written = Wt::cpp17::filesystem::last_write_time(fileName(suffix));
#ifdef WT_FILESYSTEM_IMPL_BOOST
    const auto timestamp = written + ++revision_;
#else
    const auto timestamp = written + std::chrono::seconds(++revision_);
#endif
    Wt::cpp17::filesystem::last_write_time(fileName(suffix), timestamp);
  }

private:
  std::string path_;
  unsigned revision_;
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

  const Wt::WLocale otherLocale("nl-NL");
  Wt::WMessageResourceBundle sameSource;
  sameSource.use(files.path());
  BOOST_REQUIRE_EQUAL(sameSource.resolveKey(otherLocale, "localized").value,
                      "initial");

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
  BOOST_REQUIRE_EQUAL(sameSource.resolveKey(otherLocale, "localized").value,
                      "initial");

  sameMetadata.hibernate();
  BOOST_REQUIRE_EQUAL(sameMetadata.resolveKey(locale, "localized").value,
                      "initial");

  original.hibernate();
  BOOST_REQUIRE_EQUAL(original.resolveKey(locale, "localized").value,
                      "refreshed");
  BOOST_REQUIRE_EQUAL(original.resolveKey(locale, "fallback").value, "base");
}

BOOST_AUTO_TEST_CASE( I18n_messageResourceBundleServerCacheEvents )
{
  TemporaryMessageResourceBundle files;
  files.write("_nl", "localized", "initial");
  const std::string localizedFile = files.fileName("_nl");
  const auto originalWriteTime
    = Wt::cpp17::filesystem::last_write_time(localizedFile);
  const auto originalSize = Wt::cpp17::filesystem::file_size(localizedFile);

  std::ostringstream cacheLog;
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);
  environment.server()->logger().configure("* -debug debug:WServer");
  environment.server()->logger().setStream(cacheLog);

  const auto checkConsumer = [&](const std::string& locale,
                                 const std::string& value,
                                 const std::string& event) {
    cacheLog.str("");
    Wt::WMessageResourceBundle consumer;
    consumer.use(files.path());
    BOOST_CHECK_EQUAL(consumer.resolveKey(Wt::WLocale(locale),
                                         "localized").value, value);
    BOOST_CHECK(cacheLog.str().find("message resource cache: " + event
                                   + ": " + localizedFile)
                != std::string::npos);
  };

  checkConsumer("nl-BE", "initial", "load");
  checkConsumer("nl-NL", "initial", "hit");

#ifdef WT_FILESYSTEM_IMPL_BOOST
  const auto touchedWriteTime = originalWriteTime - 60;
#else
  const auto touchedWriteTime = originalWriteTime - std::chrono::seconds(60);
#endif
  Wt::cpp17::filesystem::last_write_time(localizedFile, touchedWriteTime);
  BOOST_REQUIRE(Wt::cpp17::filesystem::last_write_time(localizedFile)
                != originalWriteTime);
  checkConsumer("nl-BE", "initial", "reload");

  files.write("_nl", "localized", "changed");
  Wt::cpp17::filesystem::last_write_time(localizedFile, touchedWriteTime);
  BOOST_REQUIRE(Wt::cpp17::filesystem::last_write_time(localizedFile)
                == touchedWriteTime);
  BOOST_REQUIRE_EQUAL(Wt::cpp17::filesystem::file_size(localizedFile),
                      originalSize);
  checkConsumer("nl-BE", "initial", "hit");
  checkConsumer("nl-NL", "initial", "hit");

  Wt::cpp17::filesystem::last_write_time(localizedFile, originalWriteTime);
  checkConsumer("nl-BE", "changed", "reload");
  checkConsumer("nl-NL", "changed", "hit");

  files.write("_nl", "localized", "<wrong>");
  checkConsumer("nl-BE", "", "reload");
  BOOST_CHECK(cacheLog.str().find("Error reading " + localizedFile)
              != std::string::npos);
  checkConsumer("nl-NL", "", "hit");
  BOOST_CHECK(cacheLog.str().find("Error reading " + localizedFile)
              != std::string::npos);

  files.write("_nl", "localized", "correct");
  checkConsumer("nl-BE", "correct", "reload");

  std::ofstream(localizedFile.c_str(), std::ios::binary | std::ios::trunc).close();
  checkConsumer("nl-BE", "", "reload");
  BOOST_CHECK(cacheLog.str().find("Error reading " + localizedFile)
              != std::string::npos);
  checkConsumer("nl-NL", "", "hit");
  BOOST_CHECK(cacheLog.str().find("Error reading " + localizedFile)
              != std::string::npos);
}

BOOST_AUTO_TEST_CASE( I18n_messageResourceBundleServerCacheSourceChanges )
{
  TemporaryMessageResourceBundle files;
  files.write("", "greeting", "base");

  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);
  const auto checkConsumer = [&](const std::string& value) {
    Wt::WMessageResourceBundle consumer;
    consumer.use(files.path());
    BOOST_CHECK_EQUAL(consumer.resolveKey(Wt::WLocale("nl-BE"),
                                         "greeting").value, value);
  };

  checkConsumer("base");
  files.write("_nl", "greeting", "Dutch");
  checkConsumer("Dutch");
  files.write("_nl-BE", "greeting", "Belgian");
  checkConsumer("Belgian");
  BOOST_REQUIRE(Wt::cpp17::filesystem::remove(files.fileName("_nl-BE")));
  checkConsumer("Dutch");
  BOOST_REQUIRE(Wt::cpp17::filesystem::remove(files.fileName("_nl")));
  checkConsumer("base");
  files.write("_nl-BE", "greeting", "returned");
  checkConsumer("returned");
}

BOOST_AUTO_TEST_CASE( I18n_messageResourceBundleServerCacheUnreadableSource )
{
  TemporaryMessageResourceBundle files;
  files.write("_nl", "greeting", "Dutch");
  BOOST_REQUIRE(Wt::cpp17::filesystem::create_directory(files.fileName("_nl-BE")));

  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);
  Wt::WMessageResourceBundle consumer;
  consumer.use(files.path());
  BOOST_CHECK_EQUAL(consumer.resolveKey(Wt::WLocale("nl-BE"),
                                        "greeting").value, "Dutch");
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
