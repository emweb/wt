// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WABSTRACTTOGGLEBUTTON_H_
#define WABSTRACTTOGGLEBUTTON_H_

#include <Wt/WFormWidget.h>
#include <Wt/WText.h>

namespace Wt {

class WLabel;

/*! \brief An abstract base class for radio buttons and check boxes.
 *
 * A toggle button provides a button with a boolean state (checked or
 * unchecked), and a text label.
 *
 * To act on a change of the state, either connect a slot to the changed()
 * signal, or connect a slot to the checked() or unChecked() signals.
 *
 * The current state (checked or unchecked) may be inspected using the
 * isChecked() method.
 */
class WT_API WAbstractToggleButton : public WFormWidget
{
protected:
  /*! \brief Creates an unchecked toggle button without label.
   */
  WAbstractToggleButton();

  /*! \brief Creates an unchecked toggle button with given text label.
   *
   * The text label is rendered to the right side of the button.
   */
  WAbstractToggleButton(const WString& text);

public:
  /*! \brief Destructor.
   */
  virtual ~WAbstractToggleButton();

  /*! \brief Sets the label text.
   *
   * The label is rendered to the right of the button.
   */
  void setText(const WString& text);

  /*! \brief Returns the label text.
   *
   * \sa setText()
   */
  const WString text() const { return text_.text; }

  /*! \brief Sets the text format.
   *
   * The textFormat controls how the string should be interpreted:
   * either as plain text, which is displayed literally, or as
   * XHTML-markup.
   *
   * When changing the textFormat to Wt::TextFormat::XHTML, and the
   * current text is literal (not created using WString::tr()), the
   * current text is parsed using an XML parser which discards
   * malicious tags and attributes silently. When the parser
   * encounters an XML parse error, the textFormat is left unchanged,
   * and this method returns false.
   *
   * Returns whether the textFormat could be set for the current text.
   *
   * The default format is Wt::TextFormat::Plain.
   */
  bool setTextFormat(TextFormat format);

  /*! \brief Returns the text format.
   *
   * \sa setTextFormat()
   */
  TextFormat textFormat() const { return text_.format; }

  /*! \brief Returns the button state.
   *
   * \sa setChecked()
   */
  bool isChecked() const { return state_ == CheckState::Checked; }

  /*! \brief Sets the button state.
   *
   * This method does not emit one of the checked() or unChecked()
   * signals.
   *
   * \sa setChecked(), setUnChecked()
   */
  void setChecked(bool checked);

  /*! \brief Checks the button.
   *
   * Does not emit the checked() signal.
   *
   * \sa setChecked(bool)
   */
  virtual void setChecked();

  /*! \brief Unchecks the button.
   *
   * Does not emit the unChecked() signal.
   *
   * \sa setChecked(bool)
   */
  virtual void setUnChecked();

  /*! \brief Returns the current value.
   *
   * Returns "yes" when checked, "maybe" when partially checked, and 
   * "no" when unchecked.
   */
  virtual WT_USTRING valueText() const override;

  /*! \brief Sets the current value.
   *
   * This interprets text values of "yes", "maybe" or "no".
   */
  virtual void setValueText(const WT_USTRING& text) override;

  /*! \brief %Signal emitted when the button gets checked.
   *
   * This signal is emitted when the user checks the button.
   *
   * You can use the changed() signal to react to any change of the
   * button state.
   */
  EventSignal<>& checked();

  /*! \brief %Signal emitted when the button gets un-checked.
   *
   * This signal is emitted when the user unchecks the button.
   *
   * You can use the changed() signal to react to any change of the
   * button state.
   */
  EventSignal<>& unChecked();

  virtual void refresh() override;
  
  /*! \brief Configures word wrapping.
   *
   * When \p wordWrap is \c true, the widget may break lines, creating a
   * multi-line text. When \p wordWrap is \c false, the text will displayed
   * on a single line, unless the text contains end-of-lines (for
   * Wt::TextFormat::Plain) or &lt;br /&gt; tags or other block-level tags
   * (for Wt::TextFormat::XHTML).
   *
   * The default value is \c false.
   *
   * \sa wordWrap()
   */
  void setWordWrap(bool wordWrap);

  /*! \brief Returns whether word wrapping is on.
   *
   * \sa setWordWrap()
   */
  bool wordWrap() const;

protected:
  CheckState state_;

  virtual void updateInput(DomElement& input, bool all) = 0;
  virtual void updateDom(DomElement& element, bool all) override;
  virtual void getFormObjects(FormObjectsMap& formObjects) override;
  virtual void setFormData(const FormData& formData) override;
  virtual void propagateRenderOk(bool deep) override;
  virtual DomElementType domElementType() const override;
  virtual bool supportsIndeterminate(const WEnvironment& env) const;
  virtual std::string formName() const override;

  virtual WStatelessSlot *getStateless(Method method) override;

private:
  static const char *CHECKED_SIGNAL;
  static const char *UNCHECKED_SIGNAL;
  static const char *UNDETERMINATE_CLICK_SIGNAL;

  WText::RichText text_;

  static const int BIT_NAKED = 0;
  static const int BIT_STATE_CHANGED = 1;
  static const int BIT_TEXT_CHANGED = 2;
  static const int BIT_WORD_WRAP_CHANGED = 3;
  static const int BIT_WORD_WRAP = 4;

  std::bitset<5> flags_;

  CheckState prevState_;

  void undoSetChecked();
  void undoSetUnChecked();
  void setCheckState(CheckState state);

  friend class WCheckBox;
  friend class WRadioButton;
  friend class WButtonGroup;
};

}

#endif // WABSTRACTTOGGLEBUTTON_H_
