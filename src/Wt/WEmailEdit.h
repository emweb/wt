// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2023 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WEMAIL_EDIT_H
#define WEMAIL_EDIT_H

#include <Wt/WEmailValidator.h>
#include <Wt/WFormWidget.h>
#include <Wt/WString.h>

#include <bitset>

namespace Wt {

/*! \class WEmailEdit Wt/WEmailEdit.h Wt/WEmailEdit.h
 *  \brief A widget for inputting email addresses
 *
 * This widget directly corresponds to an `<input type="email">`:
 *
 * - Values set with setValueText() or received from the client are automatically
 *   \link sanitize() sanitized\endlink according to the sanitization algorithm from the
 *   [HTML specification](https://html.spec.whatwg.org/multipage/input.html#email-state-(type=email)).
 * - Browsers will automatically perform validation on the input.
 * - On-screen keyboards should adjust themselves accordingly for email address input.
 * - Optionally, a \link setPattern() regular expression\endlink can be configured.
 * - It's possible to enter \link setMultiple() multiple comma-separated email addresses\endlink if configured.
 *
 * Upon construction, a WEmailValidator is automatically created and associated with the WEmailEdit. Changing any
 * of the email edit's properties, like the \link setPattern() pattern\endlink or whether
 * \link setMultiple() multiple addresses\endlink are enabled, will automatically cause the associated
 * \link emailValidator() email validator\endlink to be updated and vice versa.
 *
 * \note At the time of writing, Firefox does not do sanitization: https://bugzilla.mozilla.org/show_bug.cgi?id=1518162.
 *       This may cause the browser to add the `:invalid` pseudo tag to inputs that are deemed valid by %Wt.
 *
 * \note %Wt does not do any Punycode encoding or decoding. At the time of writing, if you put an internationalized
 *       email address into a WEmailEdit on Blink-based browsers like Google Chrome, it will be converted to Punycode
 *       by the browser. Firefox and Safari do not do this encoding, so these email addresses will be deemed invalid
 *       by the WEmailValidator.
 *
 * \sa https://developer.mozilla.org/en-US/docs/Web/HTML/Element/input/email
 * \sa https://html.spec.whatwg.org/multipage/input.html#email-state-(type=email)
 */
class WT_API WEmailEdit : public WFormWidget
{
public:
  /*! \brief Creates a new email edit.
   *
   * A default-constructed WEmailEdit will have multiple() set to `false`, and
   * the pattern() set to the empty string (i.e. no pattern).
   *
   * A default-constructed WEmailValidator will be automatically created and associated with
   * this WEmailEdit. This validator can be unset or changed using setValidator().
   */
  WEmailEdit();

  ~WEmailEdit() override;

  /*! \brief Returns the associated email validator
   *
   * If validator() is `null` or not a `WEmailValidator`, this will return `nullptr`.
   *
   * \sa setValidator()
   */
  std::shared_ptr<WEmailValidator> emailValidator();

  /*! \brief Returns the associated email validator
   *
   * If validator() is `null` or not a `WEmailValidator`, this will return `nullptr`.
   *
   * \sa setValidator()
   */
  std::shared_ptr<const WEmailValidator> emailValidator() const;

  /*! \brief Sets whether this input accepts multiple comma-separated email addresses.
   *
   * \sa multiple()
   * \sa https://developer.mozilla.org/en-US/docs/Web/HTML/Element/input/email#multiple
   */
  void setMultiple(bool multiple);

  /*! \brief Returns whether this input accepts multiple comma-separated email addresses.
   *
   * \sa setMultiple(bool multiple)
   */
  bool multiple() const noexcept { return flags_.test(BIT_MULTIPLE); }

  /*! \brief Sets a regular expression that email addresses should match
   *
   * The regular expression is an ECMAScript style regular expression.
   *
   * \sa pattern()
   * \sa https://developer.mozilla.org/en-US/docs/Web/HTML/Element/input/email#pattern
   */
  void setPattern(const WString& pattern);

  /*! \brief Returns the regular expression that email addresses should match
   *
   * \sa setPattern()
   */
  const WString& pattern() const noexcept { return pattern_; }

  /*! \brief Event signal emitted when the text in the input field changed.
   *
   * This signal is emitted whenever the text contents has
   * changed. Unlike the changed() signal, the signal is fired on
   * every change, not only when the focus is lost. Unlike the
   * keyPressed() signal, this signal is fired also for other events
   * that change the text, such as paste actions.
   *
   * \sa keyPressed(), changed()
   */
  EventSignal<>& textInput();

  /*! \brief Sets the value
   *
   * The value will be automatically \link sanitize() sanitized\endlink.
   *
   * \sa sanitize()
   */
  void setValueText(const WT_USTRING& value) override;

  /*! \brief Returns the current value as an UTF-8 string
   *
   * \sa setValueText()
   */
  WT_USTRING valueText() const override { return value_; }

  /*! \brief Sanitizes the given UTF-8 string, returning an UTF-8 string
   *
   * The sanitization is performed according to the WHATWG spec:
   *
   * - If multiple is true, all leading or trailing ASCII whitespace is removed from every email address.
   * - If multiple is false, all carriage return (`\r`) and newline (`\n`) characters are removed and
   *   all leading and trailing ASCII whitespace is removed.
   *
   * \sa https://infra.spec.whatwg.org/#strip-newlines
   * \sa https://infra.spec.whatwg.org/#strip-leading-and-trailing-ascii-whitespace
   * \sa https://html.spec.whatwg.org/multipage/input.html#email-state-(type=email)
   */
  static WT_USTRING sanitize(const WT_USTRING& input, bool multiple);

  DomElementType domElementType() const override;
  void setFormData(const FormData& formData) override;

protected:
  void render(WFlags<RenderFlag> flags) override;
  void updateDom(DomElement& element, bool all) override;
  void validatorChanged() override;

private:
  static const char *INPUT_SIGNAL;

  WString pattern_;
  WT_USTRING value_;

  static const int BIT_MULTIPLE = 0;
  static const int BIT_MULTIPLE_CHANGED = 1;
  static const int BIT_PATTERN_CHANGED = 2;
  static const int BIT_VALUE_CHANGED = 3;

  std::bitset<4> flags_;
};

}

#endif // WEMAIL_EDIT_H
