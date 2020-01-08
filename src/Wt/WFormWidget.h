// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WFORM_WIDGET_H_
#define WFORM_WIDGET_H_

#include <Wt/WInteractWidget.h>
#include <Wt/WValidator.h>

namespace Wt {

class WLabel;

/*! \class WFormWidget Wt/WFormWidget.h Wt/WFormWidget.h
 *  \brief An abstract widget that corresponds to an HTML form element.
 *
 * A %WFormWidget may receive focus (see canReceiveFocus()), can be
 * disabled, and can have a label that acts as proxy for getting
 * focus. It provides signals which reflect changes to its value, or
 * changes to its focus.
 *
 * Form widgets also have built-in support for validation, using
 * setValidator(const std::shared_ptr<WValidator> &). If the validator provide client-side
 * validation, then an invalid validation state is reflected using the
 * style class <tt>"Wt-invalid"</tt>. All validators provided by %Wt
 * implement client-side validation.
 *
 * On the server-side, use validate() method to validate the content
 * using a validator previously set.
 */
class WT_API WFormWidget : public WInteractWidget
{
public:
  /*! \brief Creates a %WFormWidget.
   */
  WFormWidget();

  /*! \brief Destructor.
   */
  ~WFormWidget();

  /*! \brief Returns the label associated with this widget.
   *
   * Returns the label (if there is one) that acts as a proxy for this
   * widget.
   *
   * \sa WLabel::setBuddy(WFormWidget *)
   */
  WLabel *label() const { return label_; }

  /*! \brief Sets the hidden state of this widget.
   *
   * If the widget has a label, it is hidden and shown together with
   * this widget.
   */
  virtual void setHidden(bool hidden,
			 const WAnimation& animation = WAnimation()) override;

  /*! \brief Returns the current value.
   *
   * This returns the current value as a string.
   */
  virtual WT_USTRING valueText() const = 0;

  /*! \brief Sets the value text.
   *
   * This sets the current value from a string value.
   */
  virtual void setValueText(const WT_USTRING& value) = 0;

  /*! \brief Sets a validator for this field.
   *
   * The validator is used to validate the current input.
   *
   * The default value is \c 0.
   *
   * \sa validate()
   */
  void setValidator(const std::shared_ptr<WValidator>& validator);

  /*! \brief Returns the validator.
   */
  virtual std::shared_ptr<WValidator> validator() const { return validator_; }

  /*! \brief Validates the field.
   *
   * \sa validated()
   */
  virtual ValidationState validate();

  /*! \brief Sets whether the widget is enabled.
   *
   * A widget that is disabled cannot receive focus or user interaction.
   *
   * This is the opposite of setDisabled().
   */
  void setEnabled(bool enabled);

  /*! \brief Sets the element read-only.
   *
   * A read-only form element cannot be edited, but the contents can
   * still be selected.
   *
   * By default, a form element area is not read-only.
   *
   * \sa setEnabled()
   */
  virtual void setReadOnly(bool readOnly);

  /*! \brief Returns whether the form element is read-only.
   *
   * \sa setReadOnly()
   */
  bool isReadOnly() const;

  /*! \brief Sets the placeholder text.
   *
   * This sets the text that is shown when the field is empty.
   */
  virtual void setPlaceholderText(const WString& placeholder);

  /*! \brief Returns the placeholder text.
   *
   * \sa setPlaceholderText()
   */
  const WString& placeholderText() const { return emptyText_; }

  /*! \brief %Signal emitted when the value was changed.
   *
   * For a keyboard input, the signal is only emitted when the focus is lost
   */
  EventSignal<>& changed();

  /*! \brief %Signal emitted when the widget is being validated.
   *
   * This signal may be useful to react to a changed validation state.
   *
   * \sa validate()
   */
  Signal<WValidator::Result>& validated() { return validated_; }

  virtual void refresh() override;

  virtual void setToolTip(const WString& text,
			  TextFormat textFormat = TextFormat::Plain)
    override;

  virtual bool canReceiveFocus() const override;
  virtual int tabIndex() const override;

protected:
  WLabel *label_;
  std::shared_ptr<WValidator> validator_;
  std::unique_ptr<JSlot> validateJs_, filterInput_, removeEmptyText_;
  WString emptyText_;

  // also used in WAbstractToggleButton
  static const char *CHANGE_SIGNAL;

  void applyEmptyText();

  virtual void enableAjax() override;

private:
  static const int BIT_ENABLED_CHANGED  = 0;
  static const int BIT_READONLY         = 1;
  static const int BIT_READONLY_CHANGED = 2;
  static const int BIT_JS_OBJECT        = 3;
  static const int BIT_VALIDATION_CHANGED = 4;
  static const int BIT_PLACEHOLDER_CHANGED = 5;

  std::bitset<6> flags_;
  Signal<WValidator::Result> validated_;
  WString validationToolTip_;

  void setLabel(WLabel *label);

  void validatorChanged();
  void defineJavaScript(bool force = false);
  void updateEmptyText();

protected:
  virtual void updateDom(DomElement& element, bool all) override;
  virtual void propagateRenderOk(bool deep) override;

  virtual void render(WFlags<RenderFlag> flags) override;

  virtual void propagateSetEnabled(bool enabled) override;

  virtual std::string formName() const;

  friend class WLabel;
  friend class WValidator;
  friend class WebSession;
};

}

#endif // WFORM_WIDGET_H_
