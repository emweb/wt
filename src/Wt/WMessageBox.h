// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WMESSAGEBOX_
#define WMESSAGEBOX_

#include <Wt/WDialog.h>

namespace Wt {

class WContainerWidget;
class WImage;
class WPushButton;
class WText;

/*! \class WMessageBox Wt/WMessageBox.h Wt/WMessageBox.h
 *  \brief A standard dialog for confirmation or to get simple user input
 *
 * The messagebox shows a message in a dialog window, with a number
 * of buttons. These buttons may be standard buttons, or customized.
 *
 * A messagebox is (usually) modal, and can be instantiated
 * synchronously or asynchronously.
 *
 * When using a messagebox asynchronously, there is no API call that
 * waits for the messagebox to be processed. Instead, the usage is
 * similar to instantiating a WDialog (or any other widget). You need
 * to connect to the buttonClicked() signal to interpret the result and
 * delete the message box.
 *
 * The synchronous use of a messagebox involves the use of the static
 * show() method, which blocks the current thread until the user has
 * processed the messabebox. Since this uses the WDialog::exec(), it
 * suffers from the same scalability issues as well as
 * limitations. See documentation of WDialog for more details.
 * 
 * \if cpp
 * Example code (using the exec() method, not recommended):
 * \code
 * StandardButton
 *   result = WMessageBox::show("Confirm", "About to wreak havoc... Continue ?",
 *                              StandardButton::Ok | StandardButton::Cancel);
 * \endcode
 * \endif
 *
 * This will show a message box that looks like this:
 *
 * <TABLE border="0" align="center"> <TR> <TD> 
 * \image html WMessageBox-default-1.png "Example of a WMessageBox (default)"
 * </TD> <TD>
 * \image html WMessageBox-polished-1.png "Example of a WMessageBox (polished)"
 * </TD> </TR> </TABLE>
 *
 * <h3>i18n</h3>
 *
 * The strings used in the WMessageBox buttons can be translated by overriding
 * the default values for the following localization keys:
 * - Wt.WMessageBox.Abort: Abort
 * - Wt.WMessageBox.Cancel: Cancel
 * - Wt.WMessageBox.Ignore: Ignore
 * - Wt.WMessageBox.No: No
 * - Wt.WMessageBox.NoToAll: No To All
 * - Wt.WMessageBox.Ok: Ok
 * - Wt.WMessageBox.Retry: Retry
 * - Wt.WMessageBox.Yes: Yes
 * - Wt.WMessageBox.YesToAll: Yes to All
 */
class WT_API WMessageBox : public WDialog
{
public:
  using WDialog::show;

  /*! \brief Creates an empty message box.
   */
  WMessageBox();

  /*! \brief Creates a message box with given caption, text, icon, and
   *         buttons.
   */
  WMessageBox(const WString& caption, const WString& text, Icon icon,
	      WFlags<StandardButton> buttons);

  /*! \brief Sets the text for the message box.
   */
  void setText(const WString& text);

  /*! \brief Returns the message box text.
   */
  const WString& text() const;

  /*! \brief Returns the text widget.
   *
   * This may be useful to customize the style or layout of the displayed
   * text.
   */
  WText *textWidget() const { return text_; }

  /*! \brief Sets the icon.
   */
  void setIcon(Icon icon);

  /*! \brief Returns the icon.
   */
  Icon icon() const { return icon_; }

  /*! \brief Adds a custom button.
   *
   * When the button is clicked, the associated result will be returned.
   */
  void addButton(std::unique_ptr<WPushButton> button, const StandardButton result);

  /*! \brief Adds a custom button with given text.
   *
   * When the button is clicked, the associated result will be returned.
   */
  WPushButton *addButton(const WString& text, StandardButton result);

  /*! \brief Adds a standard button.
   */
  WPushButton *addButton(StandardButton result);

  /*! \brief Sets standard buttons for the message box.
   */
  void setStandardButtons(WFlags<StandardButton> buttons);

  /*! \brief Returns the standard buttons.
   *
   * \sa setStandardButtons(), addButton()
   */
  WFlags<StandardButton> standardButtons() const;

  /*! \brief Returns the buttons.
   *
   * \if cpp
   * \note buttons() returning WFlags<StandardButton> has been renamed
   *       to standardButtons() in %Wt 3.3.1
   * \endif
   */
  std::vector<WPushButton *> buttons() const;

  /*! \brief Returns the button widget for the given standard button.
   *
   * Returns \c 0 if the button isn't in the message box.
   *
   * This may be useful to customize the style or layout of the button.
   */
  WPushButton *button(StandardButton button);

  /*! \brief Sets the button as the default button.
   *
   * The default button is pressed when the user presses enter. Only one
   * button can be the default button.
   *
   * If no default button is set, %Wt will take a button that is associated
   * with a Wt::StandardButton::Ok or Wt::StandardButton::Yes result.
   */
  void setDefaultButton(WPushButton *button);

  /*! \brief Sets the button as the default button.
   *
   * The default button is pressed when the user presses enter. Only one
   * button can be the default button.
   *
   * The default value is 0 (no default button).
   */
  void setDefaultButton(StandardButton button);

  /*! \brief Returns the default button.
   *
   * \sa setDefaultButton()
   */
  WPushButton *defaultButton() const { return defaultButton_; }

  /*! \brief Sets the escape button.
   *
   * The escape button is pressed when the user presses escapes.
   *
   * If no escape button is set, %Wt will take a button that is associated
   * with a Wt::StandardButton::Cancel or Wt::StandardButton::No result.
   */
  void setEscapeButton(WPushButton *button);

  /*! \brief Sets the escape button.
   *
   * The escape button is pressed when the user presses escapes.
   *
   * If no escape button is set, %Wt will take a button that is associated
   * with a Wt::StandardButton::Cancel or Wt::StandardButton::No result.
   */
  void setEscapeButton(StandardButton button);

  /*! \brief Returns the escape button.
   *
   * \sa setEscapeButton()
   */
  WPushButton *escapeButton() const { return escapeButton_; }

  /*! \brief Returns the result of this message box.
   *
   * This value is only defined after the dialog is finished.
   */
  StandardButton buttonResult() { return result_; }

  /*! \brief Convenience method to show a message box, blocking the current
   *         thread.
   *
   * Show a message box, blocking the current thread until the message box
   * is closed, and return the result. The use of this method is not
   * recommended since it uses WDialog::exec(). See documentation of
   * WDialog for detailed information.
   *
   * \if java 
   * <i>This functionality is only available on Servlet 3.0 compatible 
   * servlet containers.</i>
   * \endif
   */
  static StandardButton show(const WString& caption,
			     const WString& text,
			     WFlags<StandardButton> buttons,
			     const WAnimation& animation = WAnimation());

  /*! \brief %Signal emitted when a button is clicked.
   */
  Signal<StandardButton>& buttonClicked() { return buttonClicked_; }

  virtual void setHidden(bool hidden,
			 const WAnimation& animation = WAnimation()) override;

private:
  struct Button {
    WPushButton *button;
    StandardButton result;
  };

  std::vector<Button> buttons_;
  Icon icon_;
  StandardButton result_;
  Signal<StandardButton> buttonClicked_;
  WPushButton *defaultButton_, *escapeButton_;

  WText *text_;
  WIcon *iconW_;

  void create();

  void onFinished();
  void onButtonClick(StandardButton b);
  void mappedButtonClick(StandardButton b);

  static StandardButton order_[];
  static const char *buttonText_[];
  static WString standardButtonText(StandardButton b);
};

}

#endif // WMESSAGEBOX_
