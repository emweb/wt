/*
 * Copyright (C) 2023 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include "Wt/WApplication.h"
#include "Wt/WEmailValidator.h"

#include "Wt/Test/WTestEnvironment.h"

namespace {
  #define EMAIL_ADDRESS_PATTERN ".*@example[.]com"

  enum TestCaseFlags : int {
    Invalid = 0x0,
    Single = 0x1, // If defined, the input is a valid single email address input
    Multiple = 0x2, // If defined, the input is a valid multi email address input
    Pattern = 0x4, // If true, the input is a valid pattern-complying input
  };

  struct TestCase {
    std::string input;
    int flags = 0;
  };

  const std::vector<TestCase> cases = {
    {
      // Valid email address
      "jos@example.com", Single | Multiple | Pattern
    },
    {
      // Valid email address, but doesn't match pattern
      "jos@example.net", Single | Multiple
    },
    {
      // Valid email address, but pattern is a partial match
      "jos@example.com.be", Single | Multiple
    },
    {
      // Valid email address, special character in name
      "jo-s@example.com", Single | Multiple | Pattern
    },
    {
      // Valid email address, special character in domain
      "jos@exa-mple.com", Single | Multiple
    },
    {
      // Multiple valid email addresses
      "jos@example.com,jos.bosmans@example.com", Multiple | Pattern
    },
    {
      // Multiple valid email address, but one pattern does not match
      "jos@example.com,jos.bosmans@example.net", Multiple
    },
    {
      "greg", Invalid
    },
    {
      "@foo", Invalid
    },
    {
      "foo@", Invalid
    },
    {
      "jos.bosmans@example.com", Single | Multiple | Pattern
    },
    {
      "jos@jos@example.com", Invalid
    },
    {
      "jos@jos@example.net", Invalid
    },
    {
      // Empty entry between commas
      "jos@example.com,,jos.bosmans@example.com", Invalid
    },
    {
      // Example from MDN
      "me@example", Single | Multiple
    },
    {
      // Example from MDN
      "me@example.org", Single | Multiple
    },
    {
      // Example from MDN
      "me@example.org,you@example.org", Multiple
    },
    {
      // Example from MDN, invalid because we don't accept spaces
      "me@example.org, you@example.org", Invalid
    },
    {
      // Example from MDN, invalid because we don't accept spaces
      "me@example.org,you@example.org, us@example.org", Invalid
    },
    {
      // Example from MDN
      ",", Invalid
    },
    {
      // Example from MDN
      "me", Invalid
    },
    {
      // Example from MDN
      "me@example.org you@example.org", Invalid
    },
  };

  void testEmpty(Wt::WValidator& validator)
  {
    bool wasMandatory = validator.isMandatory();

    validator.setMandatory(false);
    {
      const auto result = validator.validate(Wt::utf8(""));
      BOOST_REQUIRE(result.message().empty());
      BOOST_REQUIRE(result.state() == Wt::ValidationState::Valid);
    }

    validator.setMandatory(true);
    {
      const auto result = validator.validate(Wt::utf8(""));
      BOOST_REQUIRE_EQUAL(result.message().toUTF8(), "This field cannot be empty");
      BOOST_REQUIRE(result.state() == Wt::ValidationState::InvalidEmpty);
    }

    validator.setMandatory(wasMandatory);
  }
}

BOOST_AUTO_TEST_CASE( WEmailValidator_validateOne )
{
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

  Wt::WEmailValidator validator;

  for (const auto& testCase : cases) {
    const auto result = validator.validate(Wt::utf8(testCase.input));
    if (testCase.flags & Single) {
      BOOST_REQUIRE(result.message().empty());
      BOOST_REQUIRE(result.state() == Wt::ValidationState::Valid);
    } else {
      BOOST_REQUIRE_EQUAL(result.message().toUTF8(), "Must be a valid email address");
      BOOST_REQUIRE(result.state() == Wt::ValidationState::Invalid);
    }
  }

  testEmpty(validator);
}

BOOST_AUTO_TEST_CASE( WEmailValidator_validateOne_withPattern )
{
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

  Wt::WEmailValidator validator;
  validator.setPattern(Wt::utf8(EMAIL_ADDRESS_PATTERN));

  for (const auto& testCase : cases) {
    const auto result = validator.validate(Wt::utf8(testCase.input));
    if ((testCase.flags & Single) && (testCase.flags & Pattern)) {
      BOOST_REQUIRE(result.message().empty());
      BOOST_REQUIRE(result.state() == Wt::ValidationState::Valid);
    } else {
      BOOST_REQUIRE_EQUAL(result.message().toUTF8(),
                          "Must be an email address matching the pattern '" EMAIL_ADDRESS_PATTERN "'");
      BOOST_REQUIRE(result.state() == Wt::ValidationState::Invalid);
    }
  }

  testEmpty(validator);
}

BOOST_AUTO_TEST_CASE( WEmailValidator_validateMultiple )
{
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

  Wt::WEmailValidator validator;
  validator.setMultiple(true);

  for (const auto& testCase : cases) {
    const auto result = validator.validate(Wt::utf8(testCase.input));
    if (testCase.flags & Multiple) {
      BOOST_REQUIRE(result.message().empty());
      BOOST_REQUIRE(result.state() == Wt::ValidationState::Valid);
    } else {
      BOOST_REQUIRE_EQUAL(result.message().toUTF8(), "Must be a comma-separated list of email addresses");
      BOOST_REQUIRE(result.state() == Wt::ValidationState::Invalid);
    }
  }

  testEmpty(validator);
}

BOOST_AUTO_TEST_CASE( WEmailValidator_validateMultiple_withPattern )
{
  Wt::Test::WTestEnvironment environment;
  Wt::WApplication app(environment);

  Wt::WEmailValidator validator;
  validator.setPattern(Wt::utf8(".*@example[.]com"));
  validator.setMultiple(true);

  for (const auto& testCase : cases) {
    const auto result = validator.validate(Wt::utf8(testCase.input));
    if ((testCase.flags & Multiple) && (testCase.flags & Pattern)) {
      BOOST_REQUIRE(result.message().empty());
      BOOST_REQUIRE(result.state() == Wt::ValidationState::Valid);
    } else {
      BOOST_REQUIRE_EQUAL(result.message().toUTF8(),
                          "Must be a comma-separated list of email addresses "
                          "matching the pattern '" EMAIL_ADDRESS_PATTERN "'");
      BOOST_REQUIRE(result.state() == Wt::ValidationState::Invalid);
    }
  }

  testEmpty(validator);
}
