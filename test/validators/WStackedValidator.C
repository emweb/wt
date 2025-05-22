/*
 * Copyright (C) 2025 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include "Wt/WApplication.h"
#include "Wt/WIntValidator.h"
#include "Wt/WLengthValidator.h"
#include "Wt/WStackedValidator.h"

#include "Wt/Test/WTestEnvironment.h"

BOOST_AUTO_TEST_CASE( WStackedValidator_insertValidator_at_index )
{
  /*
   * Tests that the WStackedValidator correcly inserts the validator
   * at the specified index.
   */
  Wt::WStackedValidator validator;
  auto v1 = std::make_shared<Wt::WIntValidator>(0, 10);
  v1->setInvalidTooLargeText("int error");
  auto v2 = std::make_shared<Wt::WLengthValidator>(1, 5);
  v2->setInvalidTooLongText("length error");
  validator.addValidator(v1);

  // Checks that the first validator is present.
  BOOST_REQUIRE(validator.size() == 1);

  // Checks that insert adds the validator.
  validator.insertValidator(0, v2);

  BOOST_REQUIRE(validator.size() == 2);

  // Checks that the inserted validator is indeed the first one.
  auto result = validator.validate("123456");
  auto expected = v2->validate("123456");

  BOOST_REQUIRE(result.state() == expected.state());
  BOOST_REQUIRE(result.message() == expected.message());
}

BOOST_AUTO_TEST_CASE( WStackedValidator_insertValidator_at_large_index )
{
  // Tests that inserting a validator at an index too large still adds it.
  Wt::WStackedValidator validator;
  auto v1 = std::make_shared<Wt::WIntValidator>(0, 10);
  auto v2 = std::make_shared<Wt::WLengthValidator>(1, 5);
  validator.addValidator(v1);
  validator.insertValidator(100, v2);

  BOOST_REQUIRE(validator.size() == 2);
}

BOOST_AUTO_TEST_CASE( WStackedValidator_insertValidator_negative_index_throws )
{
  /*
   * Tests that inserting a validator at a negative index throws an
   * exception.
   */
  Wt::WStackedValidator validator;
  auto v1 = std::make_shared<Wt::WIntValidator>(0, 10);

  BOOST_REQUIRE_THROW(validator.insertValidator(-1, v1), Wt::WException);
}

BOOST_AUTO_TEST_CASE( WStackedValidator_removeValidator_removes )
{
  Wt::WStackedValidator validator;
  auto v1 = std::make_shared<Wt::WIntValidator>(0, 10);
  validator.addValidator(v1);

  // Checks that the validator is present.
  BOOST_REQUIRE(validator.size() == 1);

  // Checks that removing the validator works.
  validator.removeValidator(v1);

  // Tests that removing a validator removes it.
  BOOST_REQUIRE(validator.size() == 0);
}

BOOST_AUTO_TEST_CASE( WStackedValidator_clear_removes_all_validators )
{
  Wt::WStackedValidator validator;
  auto v1 = std::make_shared<Wt::WIntValidator>(0, 10);
  auto v2 = std::make_shared<Wt::WLengthValidator>(1, 5);
  validator.addValidator(v1);
  validator.addValidator(v2);

  // Checks that the validators are present.
  BOOST_REQUIRE(validator.size() == 2);

  // Checks that clearing the validator removes all validators.
  validator.clear();

  // Tests that clearing the validator removes all validators.
  BOOST_REQUIRE(validator.size() == 0);
}

BOOST_AUTO_TEST_CASE( WStackedValidator_validate_returns_valid_if_all_valid )
{
  /*
   * Tests that the WStackedValidator validates a string that is valid
   * for all of its validators.
   */
  Wt::WStackedValidator validator;
  auto v1 = std::make_shared<Wt::WIntValidator>(10, 20);
  auto v2 = std::make_shared<Wt::WLengthValidator>(2, 3);
  validator.addValidator(v1);
  validator.addValidator(v2);
  auto result = validator.validate("12");

  BOOST_REQUIRE(result.state() == Wt::ValidationState::Valid);
}

BOOST_AUTO_TEST_CASE( WStackedValidator_validate_returns_first_invalid_result )
{
  /*
   * Tests that when multiple validators do consider the string invalid,
   * the WStackedValidator returns the result of the one that has the
   * lowest index.
   */
  Wt::WStackedValidator validator;
  auto v1 = std::make_shared<Wt::WIntValidator>(10, 2000);
  auto v2 = std::make_shared<Wt::WLengthValidator>(2, 3);
  auto v3 = std::make_shared<Wt::WLengthValidator>(3, 10);
  v1->setInvalidTooSmallText("too small error");
  v2->setInvalidTooLongText("too long error");
  v3->setInvalidTooShortText("too short error");
  validator.addValidator(v1);
  validator.addValidator(v2);
  validator.addValidator(v3);

  // Checks that the result of the first validator is returned.
  auto result = validator.validate("1");
  auto expected = v1->validate("1");

  BOOST_REQUIRE(result.state() == expected.state());
  BOOST_REQUIRE(result.message() == expected.message());

  // Checks that the result of the second validator is returned.
  result = validator.validate("1234");
  expected = v2->validate("1234");

  BOOST_REQUIRE(result.state() == expected.state());
  BOOST_REQUIRE(result.message() == expected.message());

  // Checks that the result of the third validator is returned.
  result = validator.validate("12");
  expected = v3->validate("12");

  BOOST_REQUIRE(result.state() == expected.state());
  BOOST_REQUIRE(result.message() == expected.message());
}

BOOST_AUTO_TEST_CASE( WStackedValidator_empty_validator_accepts_all )
{
  // Tests that an empty WStackedValidator accepts all inputs.
  Wt::WStackedValidator validator;
  auto result1 = validator.validate("");
  auto result2 = validator.validate("abc");
  auto result3 = validator.validate("123");

  BOOST_REQUIRE(result1.state() == Wt::ValidationState::Valid);
  BOOST_REQUIRE(result2.state() == Wt::ValidationState::Valid);
  BOOST_REQUIRE(result3.state() == Wt::ValidationState::Valid);
}

BOOST_AUTO_TEST_CASE( WStackedValidator_addValidator_twice_only_adds_once )
{
  // Tests that adding a validator twice does only add it once.
  Wt::WStackedValidator validator;
  auto v = std::make_shared<Wt::WIntValidator>(0, 10);
  validator.addValidator(v);
  validator.addValidator(v);

  BOOST_REQUIRE(validator.size() == 1);
}

BOOST_AUTO_TEST_CASE( WStackedValidator_insertValidator_twice_only_inserts_once )
{
  // Tests that inserting a validator twice does only add it once.
  Wt::WStackedValidator validator;
  auto v = std::make_shared<Wt::WIntValidator>(0, 10);
  validator.insertValidator(0, v);
  validator.insertValidator(0, v);

  BOOST_REQUIRE(validator.size() == 1);
}

BOOST_AUTO_TEST_CASE( WStackedValidator_single_validator_behaves_like_that_validator )
{
  /*
   * Tests that a WStackedValidator with only one validator behaves like
   * that validator.
   */

  // === TEST WITH W_INT_VALIDATOR === //

  auto intValidator = std::make_shared<Wt::WIntValidator>(5, 10);
  intValidator->setInvalidTooSmallText("too small");
  intValidator->setInvalidTooLargeText("too large");

  Wt::WStackedValidator stackedInt;
  stackedInt.addValidator(intValidator);

  // Valid input
  auto result = stackedInt.validate("7");
  auto expected = intValidator->validate("7");

  BOOST_REQUIRE(result.state() == expected.state());
  BOOST_REQUIRE(result.message() == expected.message());

  // Input too small
  result = stackedInt.validate("3");
  expected = intValidator->validate("3");

  BOOST_REQUIRE(result.state() == expected.state());
  BOOST_REQUIRE(result.message() == expected.message());

  // Input too large
  result = stackedInt.validate("20");
  expected = intValidator->validate("20");

  BOOST_REQUIRE(result.state() == expected.state());
  BOOST_REQUIRE(result.message() == expected.message());

  // Input is not an integer
  result = stackedInt.validate("abc");
  expected = intValidator->validate("abc");

  BOOST_REQUIRE(result.state() == expected.state());
  BOOST_REQUIRE(result.message() == expected.message());

  // === TEST WITH W_LENGTH_VALIDATOR === //

  auto lenValidator = std::make_shared<Wt::WLengthValidator>(2, 4);
  lenValidator->setInvalidTooShortText("too short");
  lenValidator->setInvalidTooLongText("too long");

  Wt::WStackedValidator stackedLen;
  stackedLen.addValidator(lenValidator);

  // Valid input
  result = stackedLen.validate("abc");
  expected = lenValidator->validate("abc");

  BOOST_REQUIRE(result.state() == expected.state());
  BOOST_REQUIRE(result.message() == expected.message());

  // Input too short
  result = stackedLen.validate("a");
  expected = lenValidator->validate("a");

  BOOST_REQUIRE(result.state() == expected.state());
  BOOST_REQUIRE(result.message() == expected.message());

  // Input too long
  result = stackedLen.validate("abcde");
  expected = lenValidator->validate("abcde");

  BOOST_REQUIRE(result.state() == expected.state());
  BOOST_REQUIRE(result.message() == expected.message());
}



