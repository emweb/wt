// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WREGEXPVALIDATOR_H_
#define WREGEXPVALIDATOR_H_

#include <limits>
#include <regex>

#include <Wt/WValidator.h>

namespace Wt {

#ifdef WT_TARGET_JAVA
struct RegExpFlag { };
#endif // WT_TARGET_JAVA

class WRegExp;

/*! \class WRegExpValidator Wt/WRegExpValidator.h Wt/WRegExpValidator.h
 *  \brief A validator that checks user input against a regular expression.
 *
 * This validator checks whether user input matches the given regular
 * expression. It checks the complete input; prefix ^ and suffix $ are
 * not needed.
 *
 * The regex should be specified using ECMAScript syntax
 * (http://en.cppreference.com/w/cpp/regex/ecmascript)
 *
 * Usage example:
 * \if cpp
 * \code
 * Wt::WLineEdit *lineEdit = addWidget(std::make_unique<Wt::WLineEdit>());
 * // an email address validator
 * auto validator = std::make_unique<Wt::WRegExpValidator>("[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,4}");
 * lineEdit->setValidator(validator);
 * lineEdit->setText("koen@emweb.be");
 * \endcode
 * \elseif java
 * \code
 * WLineEdit lineEdit = new WLineEdit(this);
 * // an email address validator
 * WRegExpValidator validator = new WRegExpValidator("[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,4}");
 * lineEdit.setValidator(validator);
 * lineEdit.setText("pieter@emweb.be");
 * \endcode
 * \endif
 *
 * \note This validator does not fully support unicode: it matches on the
 * CharEncoding::UTF8-encoded representation of the string.
 *
 * <h3>i18n</h3>
 *
 * The strings used in this class can be translated by overriding
 * the default values for the following localization keys:
 * - Wt.WRegExpValidator.Invalid: Invalid input
 */
class WT_API WRegExpValidator : public WValidator
{
public:
  /*! \brief Sets a new regular expression validator.
   */
  WRegExpValidator();

  /*! \brief Sets a new regular expression validator that accepts input
   *         that matches the given regular expression.
   *
   * This constructs a validator that matches the regular expression
   * \p expr.
   */
  WRegExpValidator(const WT_USTRING& pattern);

  /*! \brief Destructor.
   */
  ~WRegExpValidator();

  /*! \brief Sets the regular expression for valid input.
   *
   * Sets the ECMAscript regular expression \p expr.
   */
  void setRegExp(const WT_USTRING& pattern);

  /*! \brief Returns the regular expression for valid input.
   *
   * Returns the ECMAScript regular expression.
   */
  WT_USTRING regExpPattern() const { return pattern_; }

  /*! \brief Returns the regular expression for valid input.
   */
  std::regex regExp() const { return regex_; }

  /*! \brief Sets regular expression matching flags.
   */
  void setFlags(WFlags<RegExpFlag> flags);

  /*! \brief Returns regular expression matching flags.
   */
  WFlags<RegExpFlag> flags() const;

  /*! \brief Validates the given input.
   *
   * The input is considered valid only when it is blank for a
   * non-mandatory field, or matches the regular expression.
   */
  virtual Result validate(const WT_USTRING& input) const override;

  /*! \brief Sets the message to display when the input does not match.
   *
   * The default value is "Invalid input".
   */
  void setInvalidNoMatchText(const WString& text);

  /*! \brief Returns the message displayed when the input does not match.
   *
   * \sa setInvalidNoMatchText(const WString&)
   */
  WString invalidNoMatchText() const;

  virtual std::string javaScriptValidate() const override;

private:
  WT_USTRING pattern_;
  std::regex regex_;
  WString noMatchText_;
  static void loadJavaScript(WApplication *app);
};

}

#endif // WREGEXPVALIDATOR_H_
