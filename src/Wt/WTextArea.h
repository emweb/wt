// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WTEXTAREA_H_
#define WTEXTAREA_H_

#include <Wt/WFormWidget.h>

namespace Wt {

/*! \class WTextArea Wt/WTextArea.h Wt/WTextArea.h
 *  \brief A widget that provides a multi-line edit.
 *
 * To act upon text changes, connect a slot to the changed() signal.
 * This signal is emitted when the user changed the content, and
 * subsequently removes the focus from the line edit.
 *
 * To act upon editing, connect a slot to the keyWentUp() signal.
 *
 * At all times, the current content may be accessed with the text()
 * method.
 *
 * \if cpp
 * Usage example:
 * \code
 * auto w = std::make_unique<Wt::WContainerWidget>();
 * Wt::WLabel *label = w->addWidget(std::make_unique<Wt::WLabel>("Comments:"));
 * Wt::WTextArea *edit = w->addWidget(std::make_unique<Wt::WTextArea>(""));
 * label->setBuddy(edit);
 * \endcode
 * \endif
 *
 * %WTextArea is an \link WWidget::setInline(bool) inline \endlink widget.
 *
 * <h3>CSS</h3>
 *
 * The widget corresponds to an HTML <tt>&lt;textarea&gt;</tt> tag can be 
 * styled using inline or external CSS as appropriate.
 * The emptyText style can be configured via .Wt-edit-emptyText.
 *
 * \sa WLineEdit
 */
class WT_API WTextArea : public WFormWidget
{
public:
  /*! \brief Creates a text area with empty content and optional parent.
   */
  WTextArea();

  /*! \brief Creates a text area with given content and optional parent.
   */
  WTextArea(const WT_USTRING& content);

  /*! \brief Sets the number of columns.
   *
   * The default value is 20.
   */
  void setColumns(int cols);

  /*! \brief Sets the number of rows.
   *
   * The default value is 5.
   */
  void setRows(int rows);

  /*! \brief Returns the number of columns.
   *
   * \sa setColumns()
   */
  int columns() const { return cols_; }

  /*! \brief Returns the number of rows.
   *
   * \sa setRows()
   */
  int rows() const { return rows_; }

  /*! \brief Returns the current content.
   */
  const WT_USTRING& text() const { return content_; }

  /*! \brief Sets the content of the text area.
   *
   * The default text is "".
   */
  virtual void setText(const WT_USTRING& text);

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
  virtual void setValueText(const WT_USTRING& text) override;

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
  int      cols_, rows_;

  bool contentChanged_;
  bool attributesChanged_;

protected:
  virtual void updateDom(DomElement& element, bool all) override;
  virtual DomElementType domElementType() const override;
  virtual void propagateRenderOk(bool deep) override;

  virtual void setFormData(const FormData& formData) override;

  virtual int boxPadding(Orientation orientation) const override;
  virtual int boxBorder(Orientation orientation) const override;

  void resetContentChanged();
};

}

#endif // WTEXTAREA_H_
