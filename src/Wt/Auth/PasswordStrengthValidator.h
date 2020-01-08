// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_AUTH_PASSWORD_STRENGTH_VALIDATOR
#define WT_AUTH_PASSWORD_STRENGTH_VALIDATOR

#include <Wt/Auth/AbstractPasswordService.h>

namespace Wt {
  namespace Auth {

/*! \brief Enumeration for a password type.
 *
 * An entered password will be classified as one of these types, based
 * on the different characters used.
 *
 * The classification uses the concept of character classes, and defines
 * five classes:
 * - lower case letters
 * - upper case letters
 * - numbers
 * - other ascii characters
 * - unknowns (i.e. multi-byte UTF-8 sequences)
 *
 * For determining the number of classes, capitializaiton of the first letter
 * of a word, or appending a number, does not count as an extra class.
 */
enum PasswordStrengthType {
  /*! \brief A password with characters of only one class.
   *
   * The default minimum length required for this password type is
   * 20 characters.
   *
   * \note the default of passwdqc is Disabled
   */
  OneCharClass,

  /*! \brief A password with characters of two classes.
   *
   * The default minimum length required for this password type is
   * 15 characters.
   *
   * \note the default of passwdqc is 24 characters
   */
  TwoCharClass,

  /*! \brief A password that consists of multiple words.
   *
   * The default minimum length required for this password type is
   * 11 characters.
   *
   * \sa setMinimumPassPhraseWords()
   *
   * \note the default of passwdqc is also 11 characters
   */
  PassPhrase,     // default: 11

  /*! \brief A password with characters of three classes.
   *
   * The default minimum length required for this password type is
   * 8 characters.
   *
   * \note the default of passwdqc is also 8 characters
   */
  ThreeCharClass, // default: 8

  /*! \brief A password with characters of four classes.
   *
   * The default minimum length required for this password type is
   * 7 characters.
   *
   * \note the default of passwdqc is also 7 characters
   */
  FourCharClass   // default: 7
};

/*! \brief A default implementation for password strength validation.
 *
 * This implementation uses http://www.openwall.com/passwdqc/, a
 * password checker commonly used to validate user account passwords
 * in Linux/BSD distributions.
 *
 * The default settings are not as restrictive as those used
 * originally by passwdqc (which could be frustratingly restrictive
 * for a web application). You may want to make it change the settings
 * to demand stronger passwords for sensitive applications.
 *
 * \ingroup auth 
 */
class WT_API PasswordStrengthValidator
  : public AbstractPasswordService::AbstractStrengthValidator
{
public:
  /*! \brief Sentinel value to disable a particular check.
   */
  static const int Disabled;

  /*! \brief Default constructor.
   */
  PasswordStrengthValidator();

  /*! \brief Sets the minimum length for a password of a certain type.
   *
   * See the PasswordStrengthType documentation for defaults. You may disable
   * a password of a certain class entirely using the special value
   * Disabled.
   */
  void setMinimumLength(PasswordStrengthType type, int length);

  /*! \brief Returns the minimum length for a password of a certain type.
   *
   * \sa setMinimumLength()
   */
  int minimumLength(PasswordStrengthType type) { return minLength_[type]; }

  /*! \brief Sets the minimum number of words for a pass phrase.
   *
   * Sets the minimum number of words for a valid pass phrase.
   *
   * The default value is 3.
   *
   * \sa PassPhrase
   */
  void setMinimumPassPhraseWords(int words);

  /*! \brief Returns the minimum number of words for a pass phrase.
   *
   * \sa setMinimumPassPhraseWords()
   */
  int minimumPassPhraseWords() const { return passPhraseWords_; }

  /*! \brief Sets the minimum length for a match against a known sequence
   *         or the login name / email address
   *
   * Irrespective of other settings, a password may be checked not to
   * contain common sequences.
   *
   * This sets the minimum number of characters which is considered as
   * a match of a password against a known sequence. A lower \p length
   * setting is thus more stringent.
   *
   * The default value is 4.
   */
  void setMinimumMatchLength(int length);

  /*! \brief Returns the minimum length for a match against a known sequence.
   *
   * \sa setMinimumMatchLength()
   */
  int minimumMatchLength() const { return minMatchLength_; }

  /*! \brief Evaluates the strength of a password.
   *
   * The result is an instance of StrengthValidatorResult which 
   * contains information on the validity and the strength (0 if invalid, 
   * 5 if valid) of the password together with possible messages.
   *
   * The validator takes into account the user's login name and
   * email address, to exclude passwords that are too similar to these.
   */
  virtual AbstractPasswordService::StrengthValidatorResult 
    evaluateStrength(const WT_USTRING& password,
		     const WT_USTRING& loginName,
		     const std::string& email) const override;

private:
  int minLength_[5];
  int passPhraseWords_;
  int minMatchLength_;
};

  }
}

#endif // WT_AUTH_PASSWORD_STRENGTH_CHECKER
