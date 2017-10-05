// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WINPLACE_EDIT_H_
#define WINPLACE_EDIT_H_

#include <Wt/WCompositeWidget.h>

namespace Wt {

class WText;
class WLineEdit;
class WPushButton;

/*! \class WInPlaceEdit Wt/WInPlaceEdit.h Wt/WInPlaceEdit.h
 *  \brief A widget that provides in-place-editable text.
 *
 * The %WInPlaceEdit provides a text that may be edited in place by
 * the user by clicking on it. When clicked, the text turns into a
 * line edit, with optionally a save and cancel button (see
 * setButtonsEnabled()).
 *
 * When the user saves the edit, the valueChanged() signal is emitted.
 *
 * Usage example:
 * \if cpp
 * \code
 * auto w = std::make_unique<Wt::WContainerWidget>();
 * w->addWidget(std::make_unique<Wt::WText>("Name: "));
 * w->addWidget(std::make_unique<Wt::WInPlaceEdit>("Bob Smith"));
 * edit->setStyleClass("inplace");
 * \endcode
 * \elseif java
 * \code 
 * WContainerWidget w = new WContainerWidget();
 * new WText("Name: ", w);
 * WInPlaceEdit edit = new WInPlaceEdit("Bob Smith", w);
 * edit.setStyleClass("inplace");
 * \endcode
 * \endif
 *
 * This code will produce an edit that looks like:
 * \image html WInPlaceEdit-1.png "WInPlaceEdit text mode"
 * When the text is clicked, the edit will expand to become:
 * \image html WInPlaceEdit-2.png "WInPlaceEdit edit mode"
 *
 * <h3>CSS</h3>
 *
 * A WInPlaceEdit widget renders as a <tt>&lt;span&gt;</tt> containing
 * a WText, a WLineEdit and optional buttons (WPushButton). All these
 * widgets may be styled as such. It does not provide style information.
 *
 * In particular, you may want to provide a visual indication that the text
 * is editable e.g. using a hover effect:
 *
 * CSS stylesheet:
 * \code
 * .inplace span:hover {
 *    background-color: StandardColor::Gray;
 * }
 * \endcode
 */
class WT_API WInPlaceEdit : public WCompositeWidget
{
public:
  /*! \brief Creates an in-place edit.
   */
  WInPlaceEdit();

  /*! \brief Creates an in-place edit with the given text.
   */
  WInPlaceEdit(const WString& text);

  /*! \brief Creates an in-place edit with the given text.
   *
   * The first parameter configures whether buttons are available in edit
   * mode.
   *
   * \sa setButtonsEnabled()
   */
  WInPlaceEdit(bool buttons, const WString& text);

  /*! \brief Returns the current value.
   *
   * \sa setText()
   */
  const WString& text() const;

  /*! \brief Sets the current value.
   *
   * \sa text()
   */
  void setText(const WString& text);

  /*! \brief Sets the placeholder text.
   *
   * This sets the text that is shown when the field is empty.
   */
  void setPlaceholderText(const WString& placeholder);

  /*! \brief Returns the placeholder text.
   *
   * \sa setPlaceholderText()
   */
  const WString& placeholderText() const;

  /*! \brief Returns the line edit.
   *
   * You may use this for example to set a validator on the line edit.
   */
  WLineEdit *lineEdit() const { return edit_; }

  /*! \brief Returns the WText widget that renders the current string.
   *
   * You may use this for example to set the text format of the displayed
   * string.
   */
  WText *textWidget() const { return text_; }

  /*! \brief Returns the save button.
   *
   * This method returns \c 0 if the buttons were disabled.
   *
   * \sa cancelButton(), setButtonsEnabled()
   */
  WPushButton *saveButton() const { return save_; }

  /*! \brief Returns the cancel button.
   *
   * This method returns \c 0 if the buttons were disabled.
   *
   * \sa saveButton(), setButtonsEnabled()
   */
  WPushButton *cancelButton() const { return cancel_; }

  /*! \brief %Signal emitted when the value has been changed.
   *
   * The signal argument provides the new value.
   */
  Signal<WString>& valueChanged() { return valueChanged_; }

  /*! \brief Displays the Save and 'Cancel' button during editing
   *
   * By default, the Save and Cancel buttons are shown. Call this
   * function with \p enabled = \c false to only show a line edit.
   *
   * In this mode, the enter key or any event that causes focus to be
   * lost saves the value while the escape key cancels the editing.
   */
  void setButtonsEnabled(bool enabled = true);

protected:
  virtual void render(WFlags<RenderFlag> flags) override;

private:
  void create();
  void save();
  void cancel();

private:
  Signal<WString> valueChanged_;
  WContainerWidget *impl_, *editing_, *buttons_;
  WText *text_;
  WLineEdit *edit_;
  WPushButton *save_, *cancel_;
  WString placeholderText_;
  Wt::Signals::connection c2_;
  bool empty_;
};

}

#endif // WINPLACE_EDIT_H_
