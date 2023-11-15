#include <boost/test/unit_test.hpp>

#include <Wt/Auth/Mfa/Totp.h>

#include <Wt/WDate.h>
#include <Wt/WDateTime.h>
#include <Wt/WLogger.h>
#include <Wt/WTime.h>
#include <Wt/Utils.h>

using namespace Wt;

BOOST_AUTO_TEST_CASE( totp_test_generate_key_too_short )
{
  BOOST_CHECK_THROW(Wt::Auth::Mfa::generateSecretKey(15), Wt::WException);
}

BOOST_AUTO_TEST_CASE( totp_test_generate_key_too_long )
{
  BOOST_CHECK_THROW(Wt::Auth::Mfa::generateSecretKey(257), Wt::WException);
}

BOOST_AUTO_TEST_CASE( totp_test_generate_key_between_bounds )
{
  BOOST_TEST(Wt::Auth::Mfa::generateSecretKey(167).size() == 167);
}

BOOST_AUTO_TEST_CASE( totp_test_generate_code_too_short )
{
  BOOST_CHECK_THROW(Wt::Auth::Mfa::generateCode("", 5, std::chrono::seconds(0)), Wt::WException);
}

BOOST_AUTO_TEST_CASE( totp_test_generate_code_too_long )
{
  BOOST_CHECK_THROW(Wt::Auth::Mfa::generateCode("", 17, std::chrono::seconds(0)), Wt::WException);
}

BOOST_AUTO_TEST_CASE( totp_test_no_key_return_6_idempotent )
{
  auto code = Wt::Auth::Mfa::generateCode("", 6, std::chrono::seconds(WDateTime(WDate(2022, 1, 1), WTime(10, 10, 10)).toTime_t()));
  BOOST_REQUIRE(code == "012310");

  code = Wt::Auth::Mfa::generateCode("", 6, std::chrono::seconds(WDateTime(WDate(2022, 1, 1), WTime(10, 10, 10)).toTime_t()));
  BOOST_REQUIRE(code == "012310");
}

BOOST_AUTO_TEST_CASE( totp_test_no_key_return_6_different_period )
{
  auto code1 = Wt::Auth::Mfa::generateCode("", 6, std::chrono::seconds(WDateTime(WDate(2022, 1, 1), WTime(10, 10, 10)).toTime_t()));
  BOOST_REQUIRE(code1 == "012310");

  auto code2 = Wt::Auth::Mfa::generateCode("", 6, std::chrono::seconds(WDateTime(WDate(2022, 1, 1), WTime(10, 10, 10)).toTime_t()) + std::chrono::seconds(30));
  BOOST_REQUIRE(code1 != code2);
}

BOOST_AUTO_TEST_CASE( totp_test_key16_return_6 )
{
  std::string key = "GE2UINL2IQ4GMZZWGEZDGYL2IY======"; // From key: 15D5zD8fg6123azF
  auto code = Wt::Auth::Mfa::generateCode(key, 6, std::chrono::seconds(WDateTime(WDate(2022, 1, 1), WTime(10, 10, 10)).toTime_t()));

  BOOST_REQUIRE(code == "934872");
}

BOOST_AUTO_TEST_CASE( totp_test_key32_return_6 )
{
  std::string key = "GE2UINL2IQ4GMZZWGEZDGYL2IZTHMRRVI5UGQNSHOAYVAMJSOM3A===="; // From key: 15D5zD8fg6123azFfvF5Ghh6Gp1P12s6
  auto code = Auth::Mfa::generateCode(key, 6, std::chrono::seconds(WDateTime(WDate(2022, 1, 1), WTime(10, 10, 10)).toTime_t()));

  BOOST_REQUIRE(code == "769426");
}

BOOST_AUTO_TEST_CASE( totp_test_key64_return_6 )
{
  std::string key = "GE2UINL2IQ4GMZZWGEZDGYL2IZTHMRRVI5UGQNSHOAYVAMJSOM3EMNJUM5QWIOCBKVCDERRZGZATGNRQGRSDMYJZKFYTQUDNJVGTCYQ="; // From key: 15D5zD8fg6123azFfvF5Ghh6Gp1P12s6F54gad8AUD2F96A3604d6a9Qq8PmMM1b
  auto code = Auth::Mfa::generateCode(key, 6, std::chrono::seconds(WDateTime(WDate(2022, 1, 1), WTime(10, 10, 10)).toTime_t()));

  BOOST_REQUIRE(code == "388951");
}

BOOST_AUTO_TEST_CASE( totp_test_key16_return_8 )
{
  std::string key = "GE2UINL2IQ4GMZZWGEZDGYL2IY======"; // From key: 15D5zD8fg6123azF
  auto code = Auth::Mfa::generateCode(key, 8, std::chrono::seconds(WDateTime(WDate(2022, 1, 1), WTime(10, 10, 10)).toTime_t()));

  BOOST_REQUIRE(code == "12934872");
}

BOOST_AUTO_TEST_CASE( totp_test_key32_return_8 )
{
  std::string key = "GE2UINL2IQ4GMZZWGEZDGYL2IZTHMRRVI5UGQNSHOAYVAMJSOM3A===="; // From key: 15D5zD8fg6123azFfvF5Ghh6Gp1P12s6
  auto code = Auth::Mfa::generateCode(key, 8, std::chrono::seconds(WDateTime(WDate(2022, 1, 1), WTime(10, 10, 10)).toTime_t()));

  BOOST_REQUIRE(code == "88769426");
}

BOOST_AUTO_TEST_CASE( totp_test_key64_return_8 )
{
  std::string key = "GE2UINL2IQ4GMZZWGEZDGYL2IZTHMRRVI5UGQNSHOAYVAMJSOM3EMNJUM5QWIOCBKVCDERRZGZATGNRQGRSDMYJZKFYTQUDNJVGTCYQ="; // From key: 15D5zD8fg6123azFfvF5Ghh6Gp1P12s6F54gad8AUD2F96A3604d6a9Qq8PmMM1b
  auto code = Auth::Mfa::generateCode(key, 8, std::chrono::seconds(WDateTime(WDate(2022, 1, 1), WTime(10, 10, 10)).toTime_t()));

  BOOST_REQUIRE(code == "13388951");
}

BOOST_AUTO_TEST_CASE( totp_test_key16_return_16 )
{
  std::string key = "GE2UINL2IQ4GMZZWGEZDGYL2IY======"; // From key: 15D5zD8fg6123azF
  auto code = Auth::Mfa::generateCode(key, 16, std::chrono::seconds(WDateTime(WDate(2022, 1, 1), WTime(10, 10, 10)).toTime_t()));

  BOOST_REQUIRE(code == "0000000012934872");
}

BOOST_AUTO_TEST_CASE( totp_test_key32_return_16 )
{
  std::string key = "GE2UINL2IQ4GMZZWGEZDGYL2IZTHMRRVI5UGQNSHOAYVAMJSOM3A===="; // From key: 15D5zD8fg6123azFfvF5Ghh6Gp1P12s6
  auto code = Auth::Mfa::generateCode(key, 16, std::chrono::seconds(WDateTime(WDate(2022, 1, 1), WTime(10, 10, 10)).toTime_t()));

  BOOST_REQUIRE(code == "0000000188769426");
}

BOOST_AUTO_TEST_CASE( totp_test_key64_return_16 )
{
  std::string key = "GE2UINL2IQ4GMZZWGEZDGYL2IZTHMRRVI5UGQNSHOAYVAMJSOM3EMNJUM5QWIOCBKVCDERRZGZATGNRQGRSDMYJZKFYTQUDNJVGTCYQ="; // From key: 15D5zD8fg6123azFfvF5Ghh6Gp1P12s6F54gad8AUD2F96A3604d6a9Qq8PmMM1b
  auto code = Auth::Mfa::generateCode(key, 16, std::chrono::seconds(WDateTime(WDate(2022, 1, 1), WTime(10, 10, 10)).toTime_t()));

  BOOST_REQUIRE(code == "0000000113388951");
}

BOOST_AUTO_TEST_CASE( totp_test_validate_no_key_code_6 )
{
  std::string key = "";
  auto time = std::chrono::seconds(WDateTime(WDate(2022, 1, 1), WTime(10, 10, 10)).toTime_t());

  auto code = Auth::Mfa::generateCode(key, 6, time);

  BOOST_REQUIRE(Auth::Mfa::validateCode(key, code, 6, time));
}

BOOST_AUTO_TEST_CASE( totp_test_validate_key16_code_6 )
{
  std::string key = "GE2UINL2IQ4GMZZWGEZDGYL2IY======"; // From key: 15D5zD8fg6123azF
  auto time = std::chrono::seconds(WDateTime(WDate(2022, 1, 1), WTime(10, 10, 10)).toTime_t());

  auto code = Auth::Mfa::generateCode(key, 6, time);

  BOOST_REQUIRE(Auth::Mfa::validateCode(key, code, 6, time));
}

BOOST_AUTO_TEST_CASE( totp_test_validate_key32_code_6 )
{
  std::string key = "GE2UINL2IQ4GMZZWGEZDGYL2IZTHMRRVI5UGQNSHOAYVAMJSOM3A===="; // From key: 15D5zD8fg6123azFfvF5Ghh6Gp1P12s6
  auto time = std::chrono::seconds(WDateTime(WDate(2022, 1, 1), WTime(10, 10, 10)).toTime_t());

  auto code = Auth::Mfa::generateCode(key, 6, time);

  BOOST_REQUIRE(Auth::Mfa::validateCode(key, code, 6, time));
}

BOOST_AUTO_TEST_CASE( totp_test_validate_key64_code_6 )
{
  std::string key = "GE2UINL2IQ4GMZZWGEZDGYL2IZTHMRRVI5UGQNSHOAYVAMJSOM3EMNJUM5QWIOCBKVCDERRZGZATGNRQGRSDMYJZKFYTQUDNJVGTCYQ="; // From key: 15D5zD8fg6123azFfvF5Ghh6Gp1P12s6F54gad8AUD2F96A3604d6a9Qq8PmMM1b
  auto time = std::chrono::seconds(WDateTime(WDate(2022, 1, 1), WTime(10, 10, 10)).toTime_t());

  auto code = Auth::Mfa::generateCode(key, 6, time);

  BOOST_REQUIRE(Auth::Mfa::validateCode(key, code, 6, time));
}

BOOST_AUTO_TEST_CASE( totp_test_validate_no_key_code_8 )
{
  std::string key = "";
  auto time = std::chrono::seconds(WDateTime(WDate(2022, 1, 1), WTime(10, 10, 10)).toTime_t());

  auto code = Auth::Mfa::generateCode(key, 8, time);

  BOOST_REQUIRE(Auth::Mfa::validateCode(key, code, 8, time));
}

BOOST_AUTO_TEST_CASE( totp_test_validate_key16_code_8 )
{
  std::string key = "GE2UINL2IQ4GMZZWGEZDGYL2IY======"; // From key: 15D5zD8fg6123azF
  auto time = std::chrono::seconds(WDateTime(WDate(2022, 1, 1), WTime(10, 10, 10)).toTime_t());

  auto code = Auth::Mfa::generateCode(key, 8, time);

  BOOST_REQUIRE(Auth::Mfa::validateCode(key, code, 8, time));
}

BOOST_AUTO_TEST_CASE( totp_test_validate_key32_code_8 )
{
  std::string key = "GE2UINL2IQ4GMZZWGEZDGYL2IZTHMRRVI5UGQNSHOAYVAMJSOM3A===="; // From key: 15D5zD8fg6123azFfvF5Ghh6Gp1P12s6
  auto time = std::chrono::seconds(WDateTime(WDate(2022, 1, 1), WTime(10, 10, 10)).toTime_t());

  auto code = Auth::Mfa::generateCode(key, 8, time);

  BOOST_REQUIRE(Auth::Mfa::validateCode(key, code, 8, time));
}

BOOST_AUTO_TEST_CASE( totp_test_validate_key64_code_8 )
{
  std::string key = "GE2UINL2IQ4GMZZWGEZDGYL2IZTHMRRVI5UGQNSHOAYVAMJSOM3EMNJUM5QWIOCBKVCDERRZGZATGNRQGRSDMYJZKFYTQUDNJVGTCYQ="; // From key: 15D5zD8fg6123azFfvF5Ghh6Gp1P12s6F54gad8AUD2F96A3604d6a9Qq8PmMM1b
  auto time = std::chrono::seconds(WDateTime(WDate(2022, 1, 1), WTime(10, 10, 10)).toTime_t());

  auto code = Auth::Mfa::generateCode(key, 8, time);

  BOOST_REQUIRE(Auth::Mfa::validateCode(key, code, 8, time));
}

BOOST_AUTO_TEST_CASE( totp_test_validate_no_key_code_16 )
{
  std::string key = "";
  auto time = std::chrono::seconds(WDateTime(WDate(2022, 1, 1), WTime(10, 10, 10)).toTime_t());

  auto code = Auth::Mfa::generateCode(key, 16, time);

  BOOST_REQUIRE(Auth::Mfa::validateCode(key, code, 16, time));
}

BOOST_AUTO_TEST_CASE( totp_test_validate_key16_code_16 )
{
  std::string key = "GE2UINL2IQ4GMZZWGEZDGYL2IY======"; // From key: 15D5zD8fg6123azF
  auto time = std::chrono::seconds(WDateTime(WDate(2022, 1, 1), WTime(10, 10, 10)).toTime_t());

  auto code = Auth::Mfa::generateCode(key, 16, time);

  BOOST_REQUIRE(Auth::Mfa::validateCode(key, code, 16, time));
}

BOOST_AUTO_TEST_CASE( totp_test_validate_key32_code_16 )
{
  std::string key = "GE2UINL2IQ4GMZZWGEZDGYL2IZTHMRRVI5UGQNSHOAYVAMJSOM3A===="; // From key: 15D5zD8fg6123azFfvF5Ghh6Gp1P12s6
  auto time = std::chrono::seconds(WDateTime(WDate(2022, 1, 1), WTime(10, 10, 10)).toTime_t());

  auto code = Auth::Mfa::generateCode(key, 16, time);

  BOOST_REQUIRE(Auth::Mfa::validateCode(key, code, 16, time));
}

BOOST_AUTO_TEST_CASE( totp_test_validate_key64_code_16 )
{
  std::string key = "GE2UINL2IQ4GMZZWGEZDGYL2IZTHMRRVI5UGQNSHOAYVAMJSOM3EMNJUM5QWIOCBKVCDERRZGZATGNRQGRSDMYJZKFYTQUDNJVGTCYQ="; // From key: 15D5zD8fg6123azFfvF5Ghh6Gp1P12s6F54gad8AUD2F96A3604d6a9Qq8PmMM1b
  auto time = std::chrono::seconds(WDateTime(WDate(2022, 1, 1), WTime(10, 10, 10)).toTime_t());

  auto code = Auth::Mfa::generateCode(key, 16, time);

  BOOST_REQUIRE(Auth::Mfa::validateCode(key, code, 16, time));
}

BOOST_AUTO_TEST_CASE( totp_rfc6238 )
{
  std::string key = Wt::Utils::base32Encode("12345678901234567890");

  auto time = std::chrono::seconds(WDateTime(WDate(1970, 1, 1), WTime(0, 0, 59)).toTime_t());
  auto code = Auth::Mfa::generateCode(key, 8, time);

  BOOST_TEST(code == "94287082");

  time = std::chrono::seconds(WDateTime(WDate(2005, 3, 18), WTime(1, 58, 29)).toTime_t());
  code = Auth::Mfa::generateCode(key, 8, time);

  BOOST_REQUIRE(code == "07081804");

  time = std::chrono::seconds(WDateTime(WDate(2005, 3, 18), WTime(1, 58, 31)).toTime_t());
  code = Auth::Mfa::generateCode(key, 8, time);

  BOOST_REQUIRE(code == "14050471");

  time = std::chrono::seconds(WDateTime(WDate(2009, 2, 13), WTime(23, 31, 30)).toTime_t());
  code = Auth::Mfa::generateCode(key, 8, time);

  BOOST_REQUIRE(code == "89005924");

  time = std::chrono::seconds(WDateTime(WDate(2033, 5, 18), WTime(3, 33, 20)).toTime_t());
  code = Auth::Mfa::generateCode(key, 8, time);

  BOOST_REQUIRE(code == "69279037");

  // This test is different, due to `time_point` limits being reached.
  /*time = std::chrono::seconds(WDateTime(WDate(2603, 10, 11), WTime(11, 33, 20)).toTime_t());*/
  time = std::chrono::seconds(20'000'000'000);
  code = Auth::Mfa::generateCode(key, 8, time);

  BOOST_REQUIRE(code == "65353130");
}
