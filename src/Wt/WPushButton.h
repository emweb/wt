// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WPUSHBUTTON_H_
#define WPUSHBUTTON_H_

#include <Wt/WAnchor.h>
#include <Wt/WFormWidget.h>
#include <Wt/WJavaScript.h>
#include <Wt/WLink.h>
#include <Wt/WText.h>

namespace Wt {

/*! \class WPushButton Wt/WPushButton.h Wt/WPushButton.h
 *  \brief A widget that represents a push button.
 *
 * To act on a button click, connect a slot to the clicked() signal.
 * 
 * \if cpp
 * Usage example:
 * \code
 * // container is a WContainerWidget*
 * Wt::WPushButton *ok =
 *   container->addWidget(std::make_unique<Wt::WPushButton>("Okay"));
 * ok->clicked().connect(ok, &Wt::WPushButton::disable);
 * ok->clicked().connect(this, &MyClass::processData);
 * \endcode
 * \endif
 *
 * %WPushButton is an \link WWidget::setInline(bool) inline \endlink widget.
 *
 * <h3>CSS</h3>
 *
 * The widget corresponds to the HTML <tt>&lt;button&gt;</tt> tag (with some
 * exceptions in the bootstrap theme).
 */
class WT_API WPushButton : public WFormWidget
{
public:
  /*! \brief Creates a push button.
   */
  WPushButton();

  /*! \brief Creates a push button with given label text.
   *
   * The default text format is TextFormat::Plain.
   */
  WPushButton(const WString& text);

  /*! \brief Creates a push button with given label text.
   */
  WPushButton(const WString& text, TextFormat textFormat);

  virtual ~WPushButton();

  /*! \brief Sets the default property.
   *
   * This has only a functional meaning for a button in a dialog
   * footer, as it becomes associated with pressing 'enter' in the
   * dialog.
   *
   * A default button may be rendered in a different style, depending
   * on the theme.
   */
  void setDefault(bool enabled);

  /*! \brief Returns whether the button is a default button.
   *
   * \sa setDefault()
   */
  bool isDefault() const;

  /*! \brief Sets whether the button is checkable.
   *
   * A checkable button can be checked and unchecked, and clicking
   * will toggle between these two states.
   *
   * \sa setChecked(bool)
   */
  void setCheckable(bool checkable);

  /*! \brief Returns whether a button is checkable.
   *
   * \sa setCheckable()
   */
  bool isCheckable() const;

  /*! \brief Sets the button state.
   *
   * This is ignored for a button which is not checkable.
   *
   * This method does not emit one of the checked() or unChecked()
   * signals.
   *
   * \sa setCheckable(), setChecked(), setUnChecked()
   */
  void setChecked(bool checked);

  /*! \brief Checks the button.
   *
   * Does not emit the checked() signal.
   *
   * \sa setChecked(bool)
   */
  void setChecked();

  /*! \brief Unchecks the button.
   *
   * Does not emit the unChecked() signal.
   *
   * \sa setChecked(bool)
   */
  void setUnChecked();

  /*! \brief Returns the button state.
   *
   * \sa setChecked()
   */
  bool isChecked() const;

  /*! \brief Sets the button text.
   *
   * The default text format is Wt::TextFormat::Plain.
   *
   * When the current text format is Wt::TextFormat::XHTML, and \p text is
   * literal (not created using WString::tr()), it is parsed using an
   * XML parser which discards malicious tags and attributes
   * silently. When the parser encounters an XML parse error, the
   * textFormat is changed to Wt::TextFormat::Plain.
   *
   * Returns whether the text could be set using the current
   * textFormat. A return value of \c false indicates that the text
   * format was changed in order to be able to accept the new text.
   *
   * \sa setTextFormat()
   */
  bool setText(const WString& text);

  /*! \brief Returns the button text.
   *
   * \sa setText()
   */
  const WString& text() const { return text_.text; }

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

  /*! \brief Sets an icon.
   *
   * The icon is placed to the left of the text.
   */
  void setIcon(const WLink& link);

  /*! \brief Returns the icon.
   *
   * \sa setIcon()
   */
  WLink icon() const { return icon_; }

  /*! \brief Sets a destination link.
   *
   * This method can be used to make the button behave like a WAnchor
   * (or conversely, an anchor look like a button) and redirect to
   * another URL when clicked.
   *
   * The \p link may be to a URL, a resource, or an internal path.
   *
   * By default, a button does not link to an URL and you should
   * listen to the clicked() signal to react to a click event.
   *
   * \warning In Bootstrap theme, you should set a link before it's rendered
   *          since it commit's the button to be rendered as an anchor.
   *          (see also http://redmine.emweb.be/issues/1802).
   */
  void setLink(const WLink& link);

  /*! \brief Returns the destination link.
   *
   * \sa setLink()
   */
  const WLink& link() const { return linkState_.link; }

  /*! \brief Returns the current value.
   *
   * Returns an empty string, since a button has no value.
   */
  virtual WT_USTRING valueText() const override;

  /*! \brief Sets the current value.
   *
   * Has no effect, since a button has not value.
   */
  virtual void setValueText(const WT_USTRING& value) override;

  /*! \brief Links a popup menu to the button.
   *
   * When the button is clicked, the linked popup menu is shown.
   */
  void setMenu(std::unique_ptr<WPopupMenu> menu);

  /*! \brief Returns an associated popup menu.
   *
   * \sa setMenu()
   */
  WPopupMenu *menu() const { return popupMenu_.get(); }

  virtual void refresh() override;

  /*! \brief %Signal emitted when the button gets checked.
   *
   * This signal is emitted when the user checks the button.
   *
   * You can use the clicked() signal to react to any change of the
   * button state.
   *
   * \sa setCheckable()
   */
  EventSignal<>& checked();

  /*! \brief %Signal emitted when the button gets unchecked.
   *
   * This signal is emitted when the user unchecks the button.
   *
   * You can use the clicked() signal to react to any change of the
   * button state.
   *
   * \sa setCheckable()
   */
  EventSignal<>& unChecked();

  virtual bool setFirstFocus() override;

private:
  static const char *CHECKED_SIGNAL;
  static const char *UNCHECKED_SIGNAL;

  static const int BIT_TEXT_CHANGED = 0;
  static const int BIT_ICON_CHANGED = 1;
  static const int BIT_ICON_RENDERED = 2;
  static const int BIT_LINK_CHANGED = 3;
  static const int BIT_DEFAULT = 4;
  static const int BIT_IS_CHECKABLE = 5;
  static const int BIT_IS_CHECKED = 6;
  static const int BIT_CHECKED_CHANGED = 7;

  WAnchor::LinkState linkState_;
  WText::RichText text_;

  WLink          icon_;
  std::bitset<8> flags_;
  std::unique_ptr<WPopupMenu> popupMenu_;

protected:
  virtual void updateDom(DomElement& element, bool all) override;
  virtual DomElementType domElementType() const override;
  virtual void propagateRenderOk(bool deep) override;
  virtual void getDomChanges(std::vector<DomElement *>& result,
			     WApplication *app) override;
  virtual void propagateSetEnabled(bool enabled) override;

  virtual void enableAjax() override;

private:
  void doRedirect();
  void resourceChanged();
  void renderHRef(DomElement& href);
  void toggled();
};

}

#endif // WPUSHBUTTON_H_
