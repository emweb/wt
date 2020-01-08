// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WVALIDATOR_H_
#define WVALIDATOR_H_

#include <iostream>

#include <Wt/WObject.h>
#include <Wt/WString.h>

namespace Wt {

class WFormWidget;

/*! \brief The state in which validated input can exist.
 */
enum class ValidationState {
  Invalid,	//!< The input is invalid.
  InvalidEmpty, //!< The input is invalid (empty and mandatory).
  Valid	        //!< The input is valid.
};

/*! \class WValidator Wt/WValidator.h Wt/WValidator.h
 *  \brief A validator is used to validate user input according to
 *         pre-defined rules.
 *
 * A validator may be associated with a form widget using
 * WFormWidget::setValidator().
 *
 * The validator validates the user input. A validator may have a
 * split implementation to provide both validation at the client-side
 * (which gives instant feed-back to the user while editing), and
 * server-side validation (to be sure that the client was not tampered
 * with). The feed-back given by (client-side and server-side)
 * validation is reflected in the style class of the form field: a
 * style class of <tt>Wt-invalid</tt> is set for a field that is
 * invalid.
 *
 * This %WValidator only checks that mandatory fields are not empty.
 * This class is reimplemented in WDateValidator, WIntValidator,
 * WDoubleValidator, WLengthValidator and WRegExpValidator. All these
 * validators provide both client-side and server-side validation.
 *
 * If these validators are not suitable, you can inherit from this
 * class, and provide a suitable implementation to validate() which
 * does the server-side validation. If you want to provide client-side
 * validation for your own validator, you may also reimplement
 * javaScriptValidate().
 *
 * <h3>i18n</h3>
 *
 * The strings used in this class can be translated by overriding
 * the default values for the following localization keys:
 * - Wt.WValidator.Invalid: This field cannot be empty
 *
 * \sa WFormWidget
 */
class WT_API WValidator
{
public:
  /*! \brief Typedef for enum Wt::ValidationState */
  typedef ValidationState State;

  /*! \brief A class that holds a validation result.
   *
   * This structure is returned as the result of validation.
   */
  class WT_API Result
  {
  public:
    /*! \brief Default constructor.
     *
     * Creates an invalid result.
     */
    Result();

    /*! \brief Constructor.
     *
     * Creates a result with given \p state and \p message.
     */
    Result(ValidationState state, const WString& message);

    /*! \brief Constructor.
     *
     * Creates a result with given \p state and initalizes the message field to
     * an empty WString.
     */
    explicit Result(ValidationState state);

    /*! \brief Returns the validation state.
     */
    ValidationState state() const { return state_; }

    /*! \brief Returns the validation message.
     */
    const WString& message() const { return message_; }

  private:
    ValidationState state_;
    WString message_;
  };

  /*! \brief Creates a new validator.
   *
   * Indicate whether input is mandatory.
   *
   * \sa setMandatory(bool)
   */
  WValidator(bool mandatory = false);

  /*! \brief Destructor.
   *
   * The validator automatically removes itself from all formfields to
   * which it was associated.
   */
  virtual ~WValidator();

  /*! \brief Sets if input is mandatory
   *
   * When an input is not mandatory, then an empty field is always
   * valid.
   */
  void setMandatory(bool how);

  /*! \brief Returns if input is mandatory.
   */
  bool isMandatory() const { return mandatory_; }

  /*! \brief Sets the message to display when a mandatory field is left blank
   *
   * The default value is "This field cannot be empty".
   */
  void setInvalidBlankText(const WString& text);

  /*! \brief Returns the message displayed when a mandatory field is left blank
   *
   * \sa setInvalidBlankText(const WString&)
   */
  WString invalidBlankText() const;

  /*! \brief Validates the given input.
   *
   * \note The signature for this method changed in %Wt 3.2.0.
   */
  virtual Result validate(const WT_USTRING& input) const;

  /*! \brief Returns the validator format.
   *
   * The default implementation returns an empty string.
   */
  virtual WT_USTRING format() const;

  /*! \brief Creates a Javascript object that validates the input.
   *
   * The JavaScript expression should evaluate to an object which contains
   * a <tt>validate(text)</tt> function, which returns an object that contains
   * the following two fields:
   *  - fields: a boolean <i>valid</i>,
   *  - a \p message that indicates the problem if not valid.
   *
   * Returns an empty string if the validator does not provide a
   * client-side validation implementationq.
   *
   * \note The signature and contract changed changed in %Wt 3.1.9.
   *
   * \sa inputFilter()
   */
  virtual std::string javaScriptValidate() const;

  /*! \brief Returns a regular expression that filters input.
   *
   * The returned regular expression is used to filter keys
   * presses. The regular expression should accept valid single
   * characters.
   *
   * For details on valid regular expressions, see WRegExpValidator.
   * As an example, "[0-9]" would only accept numbers as valid input.
   *
   * The default implementation returns an empty string, which does not
   * filter any input.
   *
   * \sa javaScriptValidate()
   */
  virtual std::string inputFilter() const;

protected:
  void repaint();

private:
  bool         mandatory_;
  WString      mandatoryText_;

  std::vector<WFormWidget *> formWidgets_;

  void addFormWidget(WFormWidget *w);
  void removeFormWidget(WFormWidget *w);

  friend class WFormWidget;
};

}

#endif // WVALIDATOR_H_
