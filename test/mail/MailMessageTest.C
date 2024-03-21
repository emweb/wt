/*
 * Copyright (C) 2023 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include <Wt/WApplication.h>
#include <Wt/WDate.h>
#include <Wt/WDateTime.h>
#include <Wt/WTime.h>

#include <Wt/cpp20/tz.hpp>

#include <Wt/Mail/Message.h>

#include <Wt/Test/WTestEnvironment.h>

#include <string>
#include <web/FileUtils.h>

#include <sstream>

using namespace Wt;
using namespace Wt::Mail;

BOOST_AUTO_TEST_CASE( Message_header_RFC5322_date )
{
  // Tests whether the mail message header is in the RFC 5322 compliant
  // format. Meaning the name of the day and month are in English.
#ifdef WT_WIN32
  Wt::cpp20::date::set_install("./tzdata");
#endif
  Wt::Test::WTestEnvironment environment;
  Wt::WLocale locale = environment.locale();
  locale.setTimeZone(Wt::cpp20::date::locate_zone("GB"));
  environment.setLocale(locale);

  Wt::WApplication app(environment);
  std::string file = app.appRoot() + "types/days_months_names";
  BOOST_REQUIRE(Wt::FileUtils::exists(file + ".xml"));

  app.messageResourceBundle().use(file);

  Wt::WDateTime datetime(Wt::WDate(2022, 1, 2), Wt::WTime(10, 8, 9));
  BOOST_TEST(datetime.toString() == "Dom Ene 2 10:08:09 2022");
  BOOST_TEST(datetime.toString("dddd MMMM") == "Domigo Enero");

  std::stringstream ss;
  Message message;
  message.setDate(datetime.toLocalTime(environment.locale()));
  message.write(ss);

  auto messageString = ss.str();
  auto match = "Sun, 02 Jan 2022 10:08:09 +0000";

  BOOST_REQUIRE(messageString.find(match) != std::string::npos);
}

BOOST_AUTO_TEST_CASE( Message_header_RFC5322_to_address_no_display )
{
  Message message;
  message.addRecipient(Wt::Mail::RecipientType::To, Wt::Mail::Mailbox("test1@emweb.be"));
  std::stringstream ss;
  message.write(ss);

  auto messageString = ss.str();
  auto match = "To: <test1@emweb.be>\r\n";

  BOOST_REQUIRE(messageString.find(match) != std::string::npos);
}

BOOST_AUTO_TEST_CASE( Message_header_RFC5322_to_address_with_display )
{
  Message message;
  message.addRecipient(Wt::Mail::RecipientType::To, Wt::Mail::Mailbox("test1@emweb.be", "Test1"));
  std::stringstream ss;
  message.write(ss);

  auto messageString = ss.str();
  auto match = "To: Test1 <test1@emweb.be>\r\n";

  BOOST_REQUIRE(messageString.find(match) != std::string::npos);
}

BOOST_AUTO_TEST_CASE( Message_header_RFC5322_to_addresses_no_display )
{
  Message message;
  message.addRecipient(Wt::Mail::RecipientType::To, Wt::Mail::Mailbox("test1@emweb.be"));
  message.addRecipient(Wt::Mail::RecipientType::To, Wt::Mail::Mailbox("test2@emweb.be"));
  std::stringstream ss;
  message.write(ss);

  auto messageString = ss.str();
  auto match = "To: <test1@emweb.be>, <test2@emweb.be>\r\n";

  BOOST_REQUIRE(messageString.find(match) != std::string::npos);
}

BOOST_AUTO_TEST_CASE( Message_header_RFC5322_to_addresses_with_display )
{
  Message message;
  message.addRecipient(Wt::Mail::RecipientType::To, Wt::Mail::Mailbox("test1@emweb.be", "Test1"));
  message.addRecipient(Wt::Mail::RecipientType::To, Wt::Mail::Mailbox("test2@emweb.be"));
  std::stringstream ss;
  message.write(ss);

  auto messageString = ss.str();
  auto match = "To: Test1 <test1@emweb.be>, <test2@emweb.be>\r\n";

  BOOST_REQUIRE(messageString.find(match) != std::string::npos);
}

BOOST_AUTO_TEST_CASE( Message_header_RFC5322_cc_address_no_display )
{
  Message message;
  message.addRecipient(Wt::Mail::RecipientType::Cc, Wt::Mail::Mailbox("test1@emweb.be"));
  std::stringstream ss;
  message.write(ss);

  auto messageString = ss.str();
  auto match = "Cc: <test1@emweb.be>\r\n";

  BOOST_REQUIRE(messageString.find(match) != std::string::npos);
}

BOOST_AUTO_TEST_CASE( Message_header_RFC5322_cc_address_with_display )
{
  Message message;
  message.addRecipient(Wt::Mail::RecipientType::Cc, Wt::Mail::Mailbox("test1@emweb.be", "Test1"));
  std::stringstream ss;
  message.write(ss);

  auto messageString = ss.str();
  auto match = "Cc: Test1 <test1@emweb.be>\r\n";

  BOOST_REQUIRE(messageString.find(match) != std::string::npos);
}

BOOST_AUTO_TEST_CASE( Message_header_RFC5322_cc_addresses_no_display )
{
  Message message;
  message.addRecipient(Wt::Mail::RecipientType::Cc, Wt::Mail::Mailbox("test1@emweb.be"));
  message.addRecipient(Wt::Mail::RecipientType::Cc, Wt::Mail::Mailbox("test2@emweb.be"));
  std::stringstream ss;
  message.write(ss);

  auto messageString = ss.str();
  auto match = "Cc: <test1@emweb.be>, <test2@emweb.be>\r\n";

  BOOST_REQUIRE(messageString.find(match) != std::string::npos);
}

BOOST_AUTO_TEST_CASE( Message_header_RFC5322_cc_addresses_with_display )
{
  Message message;
  message.addRecipient(Wt::Mail::RecipientType::Cc, Wt::Mail::Mailbox("test1@emweb.be", "Test1"));
  message.addRecipient(Wt::Mail::RecipientType::Cc, Wt::Mail::Mailbox("test2@emweb.be"));
  std::stringstream ss;
  message.write(ss);

  auto messageString = ss.str();
  auto match = "Cc: Test1 <test1@emweb.be>, <test2@emweb.be>\r\n";

  BOOST_REQUIRE(messageString.find(match) != std::string::npos);
}
