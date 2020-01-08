// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WLINEEDIT_H_
#define WLINEEDIT_H_

#include <Wt/WFormWidget.h>

namespace Wt {

/*! \brief Enumeration that describes how the contents is displayed.
 *
 * \sa setEchoMode(EchoMode)
 */
enum class EchoMode {
  Normal,   //!< Characters are shown.
  Password  //!< Hide the contents as for a password.
};

/*! \brief Enumeration that describes options for input masks.
 *
 * \sa setInputMask()
 */
enum class InputMaskFlag {
  KeepMaskWhileBlurred = 0x1 //!< Keep the input mask when blurred
};

/*! \class WLineEdit Wt/WLineEdit.h Wt/WLineEdit.h
 *  \brief A widget that provides a single line edit.
 *
 * To act upon text changes, connect a slot to the changed()
 * signal. This signal is emitted when the user changed the content,
 * and subsequently removes the focus from the line edit.
 *
 * To act upon editing, connect a slot to the keyWentUp() signal because the
 * keyPressed() signal is fired before the line edit has interpreted the 
 * keypress to change its text.
 *
 * At all times, the current content may be accessed with the text()
 * method.
 *
 * You may specify a maximum length for the input using
 * setMaxLength(). If you wish to provide more detailed input
 * validation, you may set a validator using the
 * setValidator(const std::shared_ptr<WValidator> &) method. Validators provide, in general,
 * both client-side validation (as visual feed-back only) and
 * server-side validation when calling validate().
 *
 * \if cpp
 * Usage example:
 * \code
 * auto w = std::make_unique<Wt::WContainerWidget>();
 * Wt::WLabel *label = w->addWidget(std::make_unique<Wt::WLabel>("Age:"));
 * Wt::WLineEdit *edit = w->addWidget(std::make_unique<Wt::WLineEdit>("13"));
 * edit->setValidator(std::make_shared<Wt::WIntValidator>(0, 200));
 * label->setBuddy(edit);
 * \endcode
 * \endif
 *
 * The widget corresponds to the HTML <tt>&lt;input type="text"&gt;</tt> or
 * <tt>&lt;input type="password"&gt;</tt> tag.
 *
 * %WLineEdit is an \link WWidget::setInline(bool) inline \endlink widget.
 *
 * <h3>CSS</h3>
 *
 * The emptyText style can be configured via .Wt-edit-emptyText,
 * other styling can be done using inline or external CSS as appropriate.
 *
 * \sa WTextArea
 */
class WT_API WLineEdit : public WFormWidget
{
public:
  /*! \brief Creates a line edit with empty content.
   */
  WLineEdit();

  /*! \brief Creates a line edit with given content.
   */
  WLineEdit(const WT_USTRING& content);

  /*! \brief Specifies the width of the line edit in number of characters.
   *
   * This specifies the width of the line edit that is roughly
   * equivalent with \p chars characters. This does not limit the
   * maximum length of a string that may be entered, which may be set
   * using setMaxLength(int).
   *
   * The default value is 10.
   */
  void setTextSize(int chars);

  /*! \brief Returns the current width of the line edit in number of characters.
   *
   * \sa setTextSize(int)
   */
  int textSize() const { return textSize_; }

  /*! \brief Sets the content of the line edit.
   *
   * The default value is "".
   *
   * \sa text()
   */
  virtual void setText(const WT_USTRING& text);

  /*! \brief Returns the current content.
   *
   * \sa setText()
   */
  const WT_USTRING& text() const { return content_; }

  /*! \brief Returns the displayed text.
   *
   * If echoMode() is set to Normal, and no input mask is defined, this returns the same as
   * text().
   *
   * If an input mask is defined, then the text is returned including space characters.
   *
   * If echoMode() is set to Password, then a string of asterisks is returned equal to the length
   * of the text.
   *
   * \sa setText()
   */
  WT_USTRING displayText() const;

  /*! \brief Specifies the maximum length of text that can be entered.
   *
   * A value <= 0 indicates that there is no limit.
   *
   * The default value is -1.
   */
  void setMaxLength(int length);

  /*! \brief Returns the maximum length of text that can be entered.
   *
   * \sa setMaxLength(int)
   */
  int maxLength() const { return maxLength_; }

  /*! \brief Sets the echo mode.
   *
   * The default echo mode is Normal.
   */
  void setEchoMode(EchoMode echoMode);

  /*! \brief Returns the echo mode.
   *
   * \sa setEchoMode(EchoMode)
   */
  EchoMode echoMode() const { return echoMode_; }

  /*! \brief Sets (built-in browser) autocomplete support.
   *
   * Depending on the user agent, this may assist the user in filling in
   * text for common input fields (e.g. address information) based on
   * some heuristics.
   *
   * The default value is \c true.
   */
  void setAutoComplete(bool enabled);

  /*! \brief Returns auto-completion support.
   *
   * \sa setAutoComplete()
   */
  bool autoComplete() const { return autoComplete_; }

  /*! \brief Returns the current selection start.
   *
   * Returns -1 if there is no selected text.
   *
   * \sa hasSelectedText(), selectedText()
   */
  int selectionStart() const;

  /*! \brief Returns the currently selected text.
   *
   * Returns an empty string if there is currently no selected text.
   *
   * \sa hasSelectedText()
   */
  WT_USTRING selectedText() const;

  /*! \brief Returns whether there is selected text.
   *
   * \sa selectedtext()
   */
  bool hasSelectedText() const;

  /*! \brief Selects length characters starting from the start position
   *
   * \sa selectedtext()
   */
  void setSelection(int start, int length);

  /*! \brief Returns the current cursor position.
   *
   * Returns -1 if the widget does not have the focus.
   */
  int cursorPosition() const;

  /*! \brief Returns the current value.
   *
   * Returns text().
   */
  virtual WT_USTRING valueText() const override;

  /*! \brief Sets the current value.
   *
   * Calls setText().
   */
  virtual void setValueText(const WT_USTRING& value) override;

  /*! \brief Returns the input mask.
   *
   * \sa setInputMask()
   */
  WT_USTRING inputMask() const;

  /*! \brief Sets the input mask.
   *
   * If no input mask is supplied, or the given input mask
   * is empty, no input mask is applied.
   *
   * The following characters can be used in the input mask:
   * <table>
   *  <tr><th>Character</th>  <th>Description</th></tr>
   *  <tr><td>A</td>
   *      <td>ASCII alphabetic character: A-Z, a-z (required)</td></tr>
   *  <tr><td>a</td>
   *      <td>ASCII alphabetic character: A-Z, a-z (optional)</td></tr>
   *  <tr><td>N</td>
   *      <td>ASCII alphanumeric character: A-Z, a-z, 0-9 (required)</td></tr>
   *  <tr><td>n</td>
   *      <td>ASCII alphanumeric character: A-Z, a-z, 0-9 (optional)</td></tr>
   *  <tr><td>X</td><td>Any character (required)</td></tr>
   *  <tr><td>x</td><td>Any character (optional)</td></tr>
   *  <tr><td>9</td><td>Digit: 0-9 (required)</td></tr>
   *  <tr><td>0</td><td>Digit: 0-9 (optional)</td></tr>
   *  <tr><td>D</td><td>Nonzero digit: 1-9 (required)</td></tr>
   *  <tr><td>d</td><td>Nonzero digit: 1-9 (optional)</td></tr>
   *  <tr><td>#</td><td>Digit or sign: 0-9, -, + (required)</td></tr>
   *  <tr><td>H</td>
   *      <td>Hexadecimal character: A-F, a-f, 0-9 (required)</td></tr>
   *  <tr><td>h</td>
   *      <td>Hexadecimal character: A-F, a-f, 0-9 (optional)</td></tr>
   *  <tr><td>B</td><td>Binary digit: 0-1 (required)</td></tr>
   *  <tr><td>b</td><td>Binary digit: 0-1 (optional)</td></tr>
   * </table>
   * The distinction between required and optional characters won't be
   * apparent on the client side, but will affect the result of validate().
   *
   * There are also a few special characters, that won't be checked against,
   * but modify the value in some way:
   * <table>
   *  <tr><th>Character</th><th>Description</th></tr>
   *  <tr><td>&gt;</td><td>The following characters are uppercased</td></tr>
   *  <tr><td>&lt;</td><td>The following characters are lowercased</td></tr>
   *  <tr><td>!</td>
   *      <td>The casing of the following characters remains the same</td></tr>
   * </table>
   * A backslash ('\\') can be used to escape any of the mask characters
   * or modifiers, so that they can be used verbatim in the input mask.
   *
   * If the mask ends with a semicolon (';') followed by a character,
   * this character will be used on the client side to display spaces.
   * This defaults to the space (' ') character. The space character will be
   * removed from the value of this %WLineEdit.
   *
   * Examples:
   * <table>
   *  <tr><th>Input mask</th><th>Notes</th></tr>
   *  <tr><td><pre>009.009.009.009;_</pre></td>
   *      <td>IP address. Spaces are denoted by '_'. Will validate if there
   *          is at least one digit per segment.</td></tr>
   *  <tr><td><pre>9999-99-99</pre></td>
   *      <td>Date, in yyyy-MM-dd notation. Spaces are denoted by ' '.
   *          Will validate if all digits are filled in.</td></tr>
   *  <tr><td><pre>>HH:HH:HH:HH:HH:HH;_</pre></td>
   *      <td>MAC address. Spaces are denoted by '_'. Will validate if all
   *          hexadecimal characters are filled in. All characters will be
   *          formatted in uppercase.</td></tr>
   * </table>
   *
   * Input masks are enforced by JavaScript on the client side.
   * Without JavaScript or using setText(), however, non-compliant
   * strings can be entered. This does not result in an error: any
   * non-compliant characters will be removed from the input and this
   * action will be logged.
   */
  void setInputMask(const WT_USTRING &mask = "",
		    WFlags<InputMaskFlag> flags = None);

  virtual ValidationState validate() override;

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

private:
  static const char *INPUT_SIGNAL;

  WT_USTRING content_;
  WT_USTRING displayContent_;
  int        textSize_;
  int        maxLength_;
  EchoMode   echoMode_;
  bool       autoComplete_;

  static const int BIT_CONTENT_CHANGED    = 0;
  static const int BIT_TEXT_SIZE_CHANGED  = 1;
  static const int BIT_MAX_LENGTH_CHANGED = 2;
  static const int BIT_ECHO_MODE_CHANGED  = 3;
  static const int BIT_AUTOCOMPLETE_CHANGED  = 4;

  std::bitset<5> flags_;
  
  static const std::string SKIPPABLE_MASK_CHARS;

  bool maskChanged_;
  std::string  mask_;
#if !defined(WT_TARGET_JAVA)
  std::u32string inputMask_;
  std::u32string raw_;
  char32_t       spaceChar_;
#else
  std::string inputMask_;
  std::string raw_;
  char	       spaceChar_;
#endif
  WFlags<InputMaskFlag> inputMaskFlags_;
  std::string  case_;
  bool javaScriptDefined_;

  WT_USTRING removeSpaces(const WT_USTRING& text) const;
  WT_USTRING inputText(const WT_USTRING& text) const;
  void processInputMask();
#if !defined(WT_TARGET_JAVA)
  bool acceptChar(char32_t chr, size_t position) const;
#else
  bool acceptChar(char chr, size_t position) const;
#endif
  void defineJavaScript();
  void connectJavaScript(Wt::EventSignalBase&s, const std::string& methodName);
  bool validateInputMask() const;

protected:
  virtual void           updateDom(DomElement& element, bool all) override;
  virtual DomElementType domElementType() const override;
  virtual void           propagateRenderOk(bool deep) override;
  virtual void           getDomChanges(std::vector<DomElement *>& result,
                                       WApplication *app) override;
  virtual void setFormData(const FormData& formData) override;

  virtual int boxPadding(Orientation orientation) const override;
  virtual int boxBorder(Orientation orientation) const override;

  virtual void render(WFlags<RenderFlag> flags) override;
};

}

#endif // WLINEEDIT_H_
