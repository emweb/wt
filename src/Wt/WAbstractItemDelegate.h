// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2009 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WABSTRACTITEMDELEGATE_H_
#define WABSTRACTITEMDELEGATE_H_

#include <Wt/WAny.h>
#include <Wt/WObject.h>
#include <Wt/WFlags.h>
#include <Wt/WSignal.h>
#include <Wt/WValidator.h>

namespace Wt {

/*! \brief Enumeration that specifies an option for rendering a view item.
 *
 * \sa WAbstractItemDelegate::update()
 */
enum class ViewItemRenderFlag {
  Selected = 0x1,  //!< %Render as selected
  Editing = 0x2,   //!< %Render in editing mode
  Focused = 0x4,   //!< %Render (the editor) focused
  Invalid = 0x8    //!< %Render as invalid
};

W_DECLARE_OPERATORS_FOR_FLAGS(ViewItemRenderFlag)

class WAbstractItemModel;
class WWidget;
class WModelIndex;

/*! \class WAbstractItemDelegate Wt/WAbstractItemDelegate.h Wt/WAbstractItemDelegate.h
 *  \brief Abstract delegate class for rendering an item in an item view.
 *
 * Rendering of an item in a WAbstractItemView is delegated to an
 * implementation of this delegate class. The default implementation
 * used by %Wt's item views is WItemDelegate. To provide specialized
 * rendering support, you can reimplement this class (or specialize
 * WItemDelegate).
 *
 * As a delegate is used for rendering multiple items, the class should
 * not keep state about one specific item.
 *
 * A delegate may provide editing support by instantiating an editor
 * when update() is called with the Wt::ViewItemRenderFlag::Editing flag. In that
 * case, you will also need to implement editState() and
 * setEditState() to support virtual scrolling and setModelData() to
 * save the edited value to the model. For an example, see the
 * WItemDelegate.
 *
 * \sa WAbstractItemView::setItemDelegateForColumn()
 *
 * \ingroup modelview
 */
class WT_API WAbstractItemDelegate : public WObject
{
public:
  /*! \brief Constructor.
   */
  WAbstractItemDelegate();

  /*! \brief Destructor.
   */
  virtual ~WAbstractItemDelegate();

  /*! \brief Creates or updates a widget that renders an item.
   *
   * The item is specified by its model \p index, which also
   * indicates the model. If an existing widget already renders the
   * item, but needs to be updated, it is passed as the \p widget
   * parameter.
   *
   * When \p widget is \c nullptr, a new widget needs to be created and returned.
   *
   * If you want to replace the \p widget with a new one,
   * return the new widget. The old \p widget will be removed.
   * Return \c nullptr if you do not want to replace the \p widget.
   *
   * You can remove the \p widget from its parent for reuse with WWidget::removeFromParent().
   *
   * The returned widget should be a widget that responds properly to
   * be given a height, width and style class. In practice, that means
   * it cannot have a border or margin, and thus cannot be a
   * WFormWidget since those widgets typically have built-in borders
   * and margins. If you want to return a form widget (for editing the item),
   * you should wrap it in a container widget.
   *
   * The \p flags parameter indicates options for rendering the
   * item.
   */
  virtual std::unique_ptr<WWidget> update
    (WWidget *widget, const WModelIndex& index,
     WFlags<ViewItemRenderFlag> flags) = 0;

  /*! \brief Updates the model index of a widget.
   *
   * This method is invoked by the view when due to row/column insertions or
   * removals, the index has shifted.
   *
   * You should reimplement this method only if you are storing the
   * model index in the \p widget, to update the stored model index.
   *
   * The default implementation does nothing.
   */
  virtual void updateModelIndex(WWidget *widget, const WModelIndex& index);

  /*! \brief Returns the current edit state.
   *
   * Because a View may support virtual scrolling in combination with
   * editing, it may happen that the view decides to delete the editor
   * widget while the user is editing. To allow to reconstruct the editor
   * in its original state, the View will therefore ask for the editor
   * to serialize its state in a boost::any.
   *
   * When the view decides to close an editor and save its value back
   * to the model, he will first call editState() and then
   * setModelData().
   *
   * The default implementation assumes a read-only delegate, and
   * returns a boost::any().
   *
   * \sa setEditState(), setModelData()
   */
  virtual cpp17::any editState(WWidget *widget, const WModelIndex& index) const;

  /*! \brief Sets the editor data from the editor state.
   *
   * When the View scrolls back into view an item that was being edited,
   * he will use setEditState() to allow the editor to restore its current
   * editor state.
   *
   * The default implementation assumes a read-only delegate and does
   * nothing.
   *
   * \sa editState()
   */
  virtual void setEditState(WWidget *widget, const WModelIndex& index,
			    const cpp17::any& value) const;

  /*! \brief Returns whether the edited value is valid.
   *
   * The default implementation does nothing and returns Valid.
   *
   * \sa WValidator::validate()
   */
  virtual ValidationState validate(const WModelIndex& index,
				   const cpp17::any& editState) const;

  /*! \brief Saves the edited data to the model.
   *
   * The View will use this method to save the edited value to the model.
   * The \p editState is first fetched from the editor using editState().
   *
   * The default implementation assumes a read-only delegate does
   * nothing.
   */
  virtual void setModelData(const cpp17::any& editState,
			    WAbstractItemModel *model,
			    const WModelIndex& index) const;

  /*! \brief %Signal which indicates that an editor needs to be closed.
   *
   * The delegate should emit this signal when it decides for itself
   * that it should be closed (e.g. because the user confirmed the
   * edited value or cancelled the editing). The View will then rerender
   * the item if needed.
   *
   * The second boolean argument passed to the signal is a flag which
   * indicates whether the editor feels that the value should be saved or
   * cancelled.
   *
   * \sa WAbstractItemView::closeEditor()
   */
  Signal<WWidget *, bool>& closeEditor() { return closeEditor_; }

  /*! \brief %Signal which indicates that an editor needs to be closed.
   *
   * \sa closeEditor()
   */
  const Signal<WWidget *, bool>& closeEditor() const { return closeEditor_; }

private:
  Signal<WWidget *, bool> closeEditor_;
};

}

#endif // WABSTRACTITEMDELEGATE_H_
