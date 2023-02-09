// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2023 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifndef WEMAIL_VALIDATOR_H_
#define WEMAIL_VALIDATOR_H_

#include <Wt/WValidator.h>

namespace Wt {

/*! \class WEmailValidator Wt/WEmailValidator.h Wt/WEmailValidator.h
 *  \brief A basic validator for email input.
 *
 * This validator does basic email validation, to check if an email address is formed correctly
 * according to the WHATWG email input specification:
 * https://html.spec.whatwg.org/multipage/input.html#email-state-(type=email)
 *
 * This validator can also be used for \link setMultiple() multiple\endlink email addresses.
 *
 * A regex \link setPattern() pattern\endlink can be specified to check that email addresses
 * comply with this pattern.
 *
 * \note This validator only checks that the email address is correctly formed,
 *       but does not do any further checks. If you need to be sure that the
 *       email address is valid, then we recommend that you either send a confirmation
 *       email or use an email address verification service.
 */
class WT_API WEmailValidator : public WValidator {
public:
  /*! \brief Creates an email validator.
   */
  WEmailValidator();

  /*! \brief Validates the given input.
   *
   * The input is considered valid only when it is blank for a
   * non-mandatory field, or represents a properly formed email address,
   * or a list of email addresses, and matches the pattern() if non-empty.
   */
  Result validate(const WT_USTRING& input) const override;

  /*! \brief Sets the message to display when the input is not a valid email address.
   *
   * The default message is "Must be a valid email address". This string
   * is retrieved using tr("Wt.WEmailValidator.Invalid") if multiple() is `false`,
   * or tr("Wt.WEmailValidator.Invalid.Multiple") if multiple() is `true`.
   *
   * \sa invalidNotAnEmailAddressText()
   */
  void setInvalidNotAnEmailAddressText(const WString& text);

  /*! \brief Returns the message displayed when the input is not a valid email address.
   *
   * \sa setInvalidNotAnEmailAddressText(const WString& text)
   */
  WString invalidNotAnEmailAddressText() const;

  /*! \brief Sets the message to display when the input does not match the required pattern
   *
   * The default message is "Must be an email address matching the pattern '{1}'", with
   * `{1}` subsituted by the \link pattern() pattern\endlink. This string is retrieved
   * using tr("Wt.WEmailValidator.NotMaching") if multiple() is `false`, or
   * tr("Wt.WEmailValidator.NotMaching.Multiple") if multiple() is `true`.
   *
   * \sa invalidNotMatchingText()
   */
  void setInvalidNotMatchingText(const WString& text);

  /*! \brief Returns the message displayed when the input does not match the required pattern
   *
   * \sa setInvalidNotMatchingText(const WString& text)
   */
  WString invalidNotMatchingText() const;

  /*! \brief Sets whether multiple comma-separated email addresses are allowed
   *
   * \sa multiple()
   */
  void setMultiple(bool multiple);

  /*! \brief Returns whether multiple comma-separated email addresses are allowed
   *
   * \sa setMultiple(bool multiple)
   */
  bool multiple() const { return multiple_; };

  /*! \brief Sets the pattern for the input validation.
   *
   * The pattern is in ECMAScript style regex.
   *
   * \sa pattern()
   */
  void setPattern(const WString& pattern);

  /*! \brief Returns the pattern used for the input validation.
   *
   * The pattern is in ECMAScript style regex.
   *
   * \sa setPattern(WString pattern)
   */
  WString pattern() const { return pattern_; }

  std::string javaScriptValidate() const override;

private:
  WString pattern_;
  WString notAnEmailAddressText_;
  WString notMatchingText_;
  bool multiple_;

  /*! \internal
   *  \brief Validates a single email address
   *
   *  \param emailAddress an UTF-8 encoded string to be validated
   */
  bool validateOne(const std::string& emailAddress) const;

  static void loadJavaScript(WApplication *app);
};

}

#endif // WEMAIL_VALIDATOR_H_
