// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WCOMBOBOX_H_
#define WCOMBOBOX_H_

#include <Wt/WFormWidget.h>
#include <string>

namespace Wt {

/*! \class WComboBox Wt/WComboBox.h Wt/WComboBox.h
 *  \brief A widget that provides a drop-down combo-box control.
 *
 * A combo box provides the user with a set of options, from which one
 * option may be selected.
 *
 * %WComboBox is an MVC view class, using a simple string list model
 * by default. The model may be populated using 
 * addItem(const WString&) or 
 * insertItem(int, const WString&) and the contents can
 * be cleared through clear(). These methods manipulate the underlying
 * model().
 *
 * To use the combo box with a custom model instead of the default
 * WStringListModel, use setModel().
 *
 * To react to selection events, connect to the changed(), activated()
 * or sactivated() signals.
 *
 * At all times, the current selection index is available through
 * currentIndex() and the current selection text using currentText().
 *
 * WComboBox does not have support for auto-completion, this behaviour
 * can be found in the WSuggestionPopup.
 *
 * \if cpp
 * Usage example:
 * \code
 * Wt::WComboBox *combo = addWidget(std::make_unique<Wt::WComboBox>());
 * combo->addItem("Clint Eastwood");
 * combo->addItem("Mick Jagger");
 * combo->addItem("Johnny Depp");
 * combo->addItem("Kate Winslet");
 *
 * combo->setCurrentIndex(2); // Johnny Depp
 * combo->activated().connect(this, &MyWidget::comboChanged);
 * \endcode
 * \endif 
 *
 * %WComboBox is an \link WWidget::setInline(bool) inline \endlink widget.
 *
 * <h3>CSS</h3>
 *
 * The widget corresponds to the HTML <tt>&lt;select&gt;</tt> tag and
 * does not provide styling. It can be styled using inline or external
 * CSS as appropriate.
 *
 * \ingroup modelview
 */
class WT_API WComboBox : public WFormWidget
{
public:
  /*! \brief Creates an empty combo-box..
   */
  WComboBox();

  /*! \brief Adds an option item.
   *
   * This adds an item to the underlying model. This requires that the
   * model() is editable.
   *
   * Equivalent to
   * \link insertItem(int, const WString&) insertItem\endlink (count(),
   * \p text).
   */
  void addItem(const WString& text);

  /*! \brief Returns the number of items
   */
  int count() const;

  /*! \brief Returns the currently selected item.
   *
   * If no item is currently selected, the method returns -1.
   *
   * \sa setNoSelectionEnabled()
   */
  int currentIndex() const;

  /*! \brief Inserts an item at the specified position.
   *
   * The item is inserted in the underlying model at position
   * \p index. This requires that the model() is editable.
   *
   * \sa addItem(const WString&), removeItem(int)
   */
  void insertItem(int index, const WString& text);

  /*! \brief Removes the item at the specified position.
   *
   * The item is removed from the underlying model. This requires that
   * the model() is editable.
   *
   * \sa insertItem(int index, const WString&), clear()
   */
  void removeItem(int index);

  /*! \brief Changes the current selection.
   *
   * Specify a value of -1 for \p index to clear the selection.
   *
   * \note Setting a value of -1 works only if JavaScript is available.
   */
  void setCurrentIndex(int index);

  /*! \brief Changes the text for a specified option.
   *
   * The text for the item at position \p index is changed. This requires
   * that the model() is editable.
   */
  void setItemText(int index, const WString& text);

  /*! \brief Returns the text of the currently selected item.
   *
   * \sa currentIndex(), itemText(int) const
   */
  const WString currentText() const;

  /*! \brief Returns the text of a particular item.
   *
   * \sa setItemText(int, const WString&), currentText()
   */
  const WString itemText(int index) const;

  /*! \brief Sets the model to be used for the items.
   *
   * The default model is a WStringListModel.
   *
   * Items in the model can be grouped by setting the \ref
   * ItemDataRole::Level. The contents is interpreted by \ref Wt::asString, and
   * subsequent items of the same group are rendered as children of a
   * HTML <tt> <optgroup> </tt>element.
   *
   * \sa setModelColumn(int)
   */
  void setModel(const std::shared_ptr<WAbstractItemModel> model);

  /*! \brief Sets the column in the model to be used for the items.
   *
   * The column \p index in the model will be used to retrieve data.
   *
   * The default value is 0.
   *
   * \sa setModel()
   */
  void setModelColumn(int index);

  /*! \brief Returns the data model.
   *
   * \sa setModel()
   */
  std::shared_ptr<WAbstractItemModel> model() const { return model_; }

  /*! \brief Returns the index of the first item that matches a text.
   */
  int findText(const WString& text,
	       WFlags<MatchFlag> flags 
	       = MatchFlag::Exactly | MatchFlag::CaseSensitive) const;

  /*! \brief Returns the selection mode.
   *
   * Always returns SelectionMode::Single for a combo box, but may return
   * SelectionMode::Extended for a selection box
   *
   * \sa WSelectionBox::setSelectionMode()
   */
  virtual SelectionMode selectionMode() const { 
    return SelectionMode::Single;
  }

  /*! \brief Returns the current value.
   *
   * \if cpp
   * Returns currentText().
   * \else
   * Returns currentText() as a String.
   * \endif
   */
  virtual WT_USTRING valueText() const override;

  /*! \brief Sets the current value.
   *
   * Sets the current index to the item corresponding to \p value.
   */
  virtual void setValueText(const WT_USTRING& value) override;

  virtual void refresh() override;

  /*! \brief Clears all items.
   *
   * Removes all items from the underlying model. This requires that the
   * model() is editable.
   */
  void clear();

  /*! \brief %Signal emitted when the selection changed.
   *
   * The newly selected item is passed as an argument.
   *
   * \sa sactivated(), currentIndex()
   */
  Signal<int>& activated() { return activated_; }

  /*! \brief %Signal emitted when the selection changed.
   *
   * The newly selected text is passed as an argument.
   *
   * \sa activated(), currentText()
   */
  Signal<WString>& sactivated() { return sactivated_; }

  /*! \brief Enables the ability to have 'no currently selected' item.
   *
   * The setting may only be changed for a combo box (and not for a
   * selection box). When enabled, the currentIndex() may be '-1' also
   * when the combo box contains values. The user can however not
   * select this option, it is thus only useful as a default value.
   *
   * By default, no selection is \c false for a combo-box and \c true
   * for a selection box.
   */
  void setNoSelectionEnabled(bool enabled);

  /*! \brief Returns whether 'no selection' is a valid state.
   *
   * \sa setNoSelectionEnabled()
   */
  bool noSelectionEnabled() const { return noSelectionEnabled_; }

private:
  std::shared_ptr<WAbstractItemModel> model_;
  int modelColumn_;
  int currentIndex_;
  void *currentIndexRaw_;

  bool itemsChanged_;
  bool selectionChanged_;
  bool currentlyConnected_;
  bool noSelectionEnabled_;

  std::vector<Wt::Signals::connection> modelConnections_;

  Signal<int> activated_;
  Signal<WString> sactivated_;

  void layoutChanged();
  void itemsChanged();
  void propagateChange();

  void rowsInserted(const WModelIndex &index, int from, int to);
  void rowsRemoved(const WModelIndex &index, int from, int to);
  void saveSelection();
  void restoreSelection();

  virtual bool supportsNoSelection() const;

protected:
  virtual void updateDom(DomElement& element, bool all) override;
  virtual DomElementType domElementType() const override;
  virtual void propagateRenderOk(bool deep) override;

  virtual void setFormData(const FormData& formData) override;

  virtual bool isSelected(int index) const;

  friend class WSelectionBox;

private:
  void makeCurrentIndexValid();
};

}

#endif // WCOMBOBOX_H_
