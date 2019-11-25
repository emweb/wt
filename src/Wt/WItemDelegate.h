// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2009 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WITEMDELEGATE_H_
#define WITEMDELEGATE_H_

#include <Wt/WAbstractItemDelegate.h>
#include <Wt/WCheckBox.h>
#include <Wt/WLineEdit.h>
#include <Wt/WString.h>

namespace Wt {
  class WAnchor;
  class WCheckBox;
  class WContainerWidget;
  class WImage;
  class WLineEdit;
  class WText;

#ifndef WT_CNOR
  template <class Widget> class IndexEdit;
  typedef IndexEdit<WCheckBox> IndexCheckBox;
  typedef IndexEdit<WContainerWidget> IndexContainerWidget;
  typedef IndexEdit<WAnchor> IndexAnchor;
  typedef IndexEdit<WText> IndexText;
#else
  class IndexCheckBox;
  class IndexContainerWidget;
  class IndexAnchor;
  class IndexText;
#endif // WT_CNOR

/*! \class WItemDelegate Wt/WItemDelegate.h Wt/WItemDelegate.h
 *  \brief Standard delegate class for rendering a view item.
 *
 * This class provides the standard implementation for rendering an
 * item (as in a WAbstractItemView), and renders data provided by the
 * standard data roles (see ItemDataRole). It also provides default
 * editing support using a line edit.
 *
 * You may provide special editing support for an item by specializing
 * the widget and reimplement createEditor(), setModelData(),
 * editState(), and setEditState().
 *
 * \ingroup modelview
 */
class WT_API WItemDelegate : public WAbstractItemDelegate
{
public:
  /*! \brief Create an item delegate.
   */
  WItemDelegate();

  /*! \brief Creates or updates a widget that renders an item.
   *
   * The following properties of an item are rendered:
   *
   * - text using the Wt::ItemDataRole::Display data, with the format specified
   *   by setTextFormat()
   * - a check box depending on the Wt::ItemFlag::UserCheckable flag and
   *   Wt::ItemDataRole::Checked data
   * - an anchor depending on the value of Wt::ItemDataRole::Link
   * - an icon depending on the value of Wt::ItemDataRole::Decoration
   * - a tooltip depending on the value of Wt::ItemDataRole::ToolTip
   * - a custom style class depending on the value of Wt::ItemDataRole::StyleClass
   *
   * When the flags indicates Wt::ViewItemRenderFlag::Editing, then createEditor() is
   * called to create a suitable editor for editing the item.
   */
  virtual std::unique_ptr<WWidget> update
    (WWidget *widget, const WModelIndex& index,
     WFlags<ViewItemRenderFlag> flags) override;

  virtual void updateModelIndex(WWidget *widget, const WModelIndex& index)
    override;

  /*! \brief Sets the text format string.
   *
   * \if cpp
   *
   * The ItemDataRole::Display data is converted to a string using asString() by passing
   * the given format.
   *
   * \elseif java
   *
   * The ItemDataRole::Display data is converted to a string using {javadoclink
   * StringUtils#asString(Object)}, passing the given format. If the format is
   * an empty string, this corresponds to {javadoclink Object#toString()}.
   *
   * \endif 
   *
   * The default value is "".
   */
  void setTextFormat(const WT_USTRING& format);

  /*! \brief Returns the text format string.
   *
   * \sa setTextFormat()
   */
  const WT_USTRING& textFormat() const { return textFormat_; }

  /*! \brief Saves the edited data to the model.
   *
   * The default implementation saves the current edit value to the model.
   * You will need to reimplement this method for a custom editor.
   *
   * As an example of how to deal with a specialized editor, consider the
   * default implementation:
   * \if cpp
   * \code
   * void WItemDelegate::setModelData(const Wt::cpp17::any& editState,
   *                                  Wt::WAbstractItemModel *model,
   *                                  const Wt::WModelIndex& index) const
   * {
   *   model->setData(index, editState, ItemDataRole::Edit);
   * }
   * \endcode
   * \elseif java
   * \code
   * public void setModelData(Object editState, WAbstractItemModel model, WModelIndex index) {
   *   model.setData(index, editState, ItemDataRole.ItemDataRole::Edit);
   * }
   * \endcode
   * \endif
   *
   * \sa createEditor(), editState()
   */
  virtual void setModelData(const cpp17::any& editState,
			    WAbstractItemModel *model,
			    const WModelIndex& index) const override;

  /*! \brief Returns the current edit state.
   *
   * The default implementation returns the current text in the line edit.
   * You will need to reimplement this method for a custom editor.
   *
   * As an example of how to deal with a specialized editor, consider the
   * default implementation:
   * \if cpp
   * \code
   * cpp17::any WItemDelegate::editState(Wt::WWidget *editor) const
   * {
   *   Wt::WContainerWidget *w = dynamic_cast<Wt::WContainerWidget *>(editor);
   *   Wt::WLineEdit *lineEdit = dynamic_cast<Wt::WLineEdit *>(w->widget(0));
   *
   *   return cpp17::any(lineEdit->text());
   * }
   * \endcode
   * \elseif java
   * \code
   * public Object getEditState(WWidget editor) {
   *   WContainerWidget w = (WContainerWidget) editor;
   *   WLineEdit lineEdit = (WLineEdit) w.getWidget(0);
   *   return lineEdit.getText();
   * }
   * \endcode
   * \endif
   *
   * \sa createEditor(), setEditState(), setModelData()
   */
  virtual cpp17::any editState(WWidget *editor, const WModelIndex& index) const
    override;

  /*! \brief Sets the editor data from the editor state.
   *
   * The default implementation resets the text in the line edit.
   * You will need to reimplement this method if for a custom editor.
   *
   * As an example of how to deal with a specialized editor, consider the
   * default implementation:
   * \if cpp
   * \code
   * void WItemDelegate::setEditState(Wt::WWidget *editor, const WModelIndex& index, const cpp17::any& value) const
   * {
   *   Wt::WContainerWidget *w = dynamic_cast<Wt::WContainerWidget *>(editor);
   *   Wt::WLineEdit *lineEdit = dynamic_cast<Wt::WLineEdit *>(w->widget(0));
   *
   *   lineEdit->setText(cpp17::any_cast<Wt::WString>(value));
   * }
   * \endcode
   * \elseif java
   * \code
   * public void setEditState(WWidget editor, WModelIndex index, Object value) {
   *   WContainerWidget w = (WContainerWidget) editor;
   *   WLineEdit lineEdit = (WLineEdit) w.getWidget(0);
   *   lineEdit.setText((String) value);
   * }
   * \endcode
   * \endif
   *
   * \sa createEditor()
   */
  virtual void setEditState(WWidget *editor, const WModelIndex& index,
			    const cpp17::any& value) const
    override;

protected:
  /*! \brief Creates an editor for a data item.
   *
   * The default implementation returns a WLineEdit which edits the
   * item's Wt::ItemDataRole::Edit value.
   *
   * You may reimplement this method to provide a suitable editor, or
   * to attach a custom validator. In that case, you will probably
   * also want to reimplement editState(), setEditState(), and
   * setModelData().
   *
   * The editor should not keep a reference to the model index (it
   * does not need to since setModelData() will provide the proper
   * model index to save the data to the model). Otherwise, because
   * model indexes may shift because of row or column insertions, you
   * should reimplement updateModelIndex().
   *
   * As an example of how to provide a specialized editor, consider the
   * default implementation, which returns a WLineEdit:
   * \if cpp
   * \code
   * std::unique_ptr<Wt::WWidget> WItemDelegate::createEditor(const Wt::WModelIndex& index, WFlags<ViewItemRenderFlag> flags) const
   * {
   *   auto result = std::make_unique<Wt::WContainerWidget>();
   *   result->setSelectable(true);
   *
   *   auto lineEdit = std::make_unique<Wt::WLineEdit>();
   *   lineEdit->setText(asString(index.data(ItemDataRole::Edit), textFormat_));
   *   lineEdit->enterPressed().connect(std::bind(&WItemDelegate::doCloseEditor, this, result, true));
   *   lineEdit->escapePressed().connect(std::bind(&WItemDelegate::doCloseEditor, this, result, false));
   *
   *   if (flags.test(ViewItemRenderFlag::Focused))
   *     lineEdit->setFocus();
   *
   *   // We use a layout so that the line edit fills the entire cell.
   *   result->setLayout(std::make_unique<WHBoxLayout>());
   *   result->layout()->setContentsMargins(1, 1, 1, 1);
   *   result->layout()->addWidget(std::move(lineEdit));
   *
   *   return result;
   * }
   *
   * void WItemDelegate::doCloseEditor(Wt::WWidget *editor, bool save) const
   * {
   *   closeEditor().emit(editor, save);
   * }
   * \endcode
   * \elseif java
   * \code
   * protected WWidget createEditor(WModelIndex index, EnumSet&lt;ViewItemRenderFlag&gt; flags) {
   *  final WContainerWidget result = new WContainerWidget();
   *  result.setSelectable(true);
   *  WLineEdit lineEdit = new WLineEdit();
   *  lineEdit.setText(StringUtils.asString(index.getData(ItemDataRole.ItemDataRole::Edit), this.textFormat_).toString());
   *  lineEdit.enterPressed().addListener(this, new Signal.Listener() {
   *    public void trigger() {
   *      WItemDelegate.this.closeEditor().trigger(result, true);
   *    }
   *  });
   *  lineEdit.escapePressed().addListener(this, new Signal.Listener() {
   *    public void trigger() {
   *      WItemDelegate.this.closeEditor().trigger(result, false);
   *    }
   *  });
   *
   *  if (flags.contains(ViewItemRenderFlag.ViewItemRenderFlag::Focused))
   *    lineEdit.setFocus();
   *
   *  result.setLayout(new WHBoxLayout());
   *  result.getLayout().setContentsMargins(1, 1, 1, 1);
   *  result.getLayout().addWidget(lineEdit);
   *  return result;
   * }
   * \endcode
   * \endif
   */
  virtual std::unique_ptr<WWidget>
    createEditor(const WModelIndex& index,
		 WFlags<ViewItemRenderFlag> flags) const;

private:
  WT_USTRING textFormat_;

  struct WidgetRef {
    std::unique_ptr<WWidget> created;
    WWidget *w;
    WidgetRef(WWidget *widget) : w(widget) { }
  };

  IndexCheckBox *checkBox(WidgetRef& w, const WModelIndex& index,
			 bool autoCreate, bool update = false, bool triState = false);

  IndexText *textWidget(WidgetRef& w, const WModelIndex& index);
  WImage *iconWidget(WidgetRef& w, const WModelIndex& index, bool autoCreate = false);
  IndexAnchor *anchorWidget(WidgetRef& w, const WModelIndex& index, bool autoCreate = false);

  void onCheckedChange(IndexCheckBox *checkBox) const;
  void doCloseEditor(WWidget *editor, bool save) const;
};

}

#endif // WITEMDELEGATE_H_
