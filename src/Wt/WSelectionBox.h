// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WSELECTIONBOX_H_
#define WSELECTIONBOX_H_

#include <Wt/WComboBox.h>

namespace Wt {

/*! \class WSelectionBox Wt/WSelectionBox.h Wt/WSelectionBox.h
 *  \brief A selection box allows selection from a list of options.
 *
 * By default, a selection box may be used to let the user select one
 * item from a list. This may be changed to multiple selection mode
 * using setSelectionMode().
 *
 * The current selection may be set and read using setCurrentIndex()
 * and currentIndex(), for \link Wt::SelectionMode::Single
 * SelectionMode::Single\endlink mode, or setSelectedIndexes() and
 * selectedIndexes() for Wt::SelectionMode::Extended mode. 
 * The activated() and sactivated()
 * signals are not emited in the Wt::SelectionMode::Extended mode, use the
 * changed() signal.
 *
 * %WSelectionBox is an MVC view class, using a simple string list
 * model by default. The model may be populated using WComboBox::addItem() 
 * or WComboBox::insertItem() and the contents can 
 * be cleared through clear(). These methods manipulate the underlying
 * model().
 *
 * To use the selectionbox with a custom model instead of the default
 * WStringListModel, use setModel().
 *
 * \if cpp
 * Usage example:
 * \code
 * Wt::WSelectionBox *selectionBox = addWidget(std::make_unique<Wt::WSelectionBox>());
 * selectionBox->addItem("Clint Eastwood");
 * selectionBox->addItem("Mick Jagger");
 * selectionBox->addItem("Johnny Depp");
 * selectionBox->addItem("Kate Winslet");
 *
 * selectionBox->setCurrentIndex(2); // Johnny Depp
 * selectionBox->activated().connect(this, &MyWidget::comboChanged);
 * \endcode
 * \endif
 *
 * %WSelectionBox is an \link WWidget::setInline(bool) inline \endlink widget.
 *
 * <h3>CSS</h3>
 *
 * The widget corresponds to the HTML <tt>&lt;select&gt;</tt> tag and
 * does not provide styling. It can be styled using inline or external
 * CSS as appropriate.
 *
 * \ingroup modelview
 */
class WT_API WSelectionBox : public WComboBox
{
public:
  /*! \brief Constructor.
   */
  WSelectionBox();

  /*! \brief Returns the number of items that are visible.
   */
  int verticalSize() const { return verticalSize_; }

  /*! \brief Sets the number of items that are visible.
   *
   * If more items are available, a scroll-bar is provided.
   */
  void setVerticalSize(int items);

  /*! \brief Sets the selection mode.
   *
   * The default selection mode is SelectionMode::Single. You can change to
   * Wt::SelectionMode::Extended to allow selection of multiple items.
   */
  void setSelectionMode(SelectionMode mode);

  /*! \brief Returns the selection mode.
   *
   * \sa setSelectionMode(SelectionMode)
   */
  virtual SelectionMode selectionMode() const override { return selectionMode_; }

  /*! \brief Returns the current selection (in Wt::SelectionMode::Extended mode).
   *
   * Get the list of currently selected items. This method is only defined
   * when selectionMode() is Wt::SelectionMode::Extended. Otherwise, you should use
   * currentIndex() to get item currently selected.
   *
   * \sa currentIndex()
   */
  const std::set<int>& selectedIndexes() const;

  /*! \brief Sets the selection (in Wt::SelectionMode::Extended mode).
   *
   * For an Wt::SelectionMode::Extended mode, set the list of currently selected
   * items.
   *
   * \sa selectedIndexes()
   */
  void setSelectedIndexes(const std::set<int>& selection);

  /*! \brief Clears the current selection.
   *
   * Clears the current selection.
   *
   * \sa setCurrentIndex(), setSelectedIndexes()
   */
  void clearSelection();

private:
  int           verticalSize_;
  SelectionMode selectionMode_;
  std::set<int> selection_;

  bool configChanged_;

  virtual bool supportsNoSelection() const override;

protected:
  virtual void updateDom(DomElement& element, bool all) override;
  virtual void setFormData(const FormData& formData) override;
  virtual void propagateRenderOk(bool deep) override;

  virtual bool isSelected(int index) const override;
};

}

#endif // WSELECTIONBOX_
