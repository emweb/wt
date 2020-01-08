/*
 * Copyright (C) 2009 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WItemDelegate.h"

#include "Wt/WAbstractItemModel.h"
#include "Wt/WAnchor.h"
#include "Wt/WApplication.h"
#include "Wt/WCheckBox.h"
#include "Wt/WContainerWidget.h"
#include "Wt/WEnvironment.h"
#include "Wt/WImage.h"
#include "Wt/WModelIndex.h"
#include "Wt/WHBoxLayout.h"
#include "Wt/WText.h"
#include "Wt/WTheme.h"

namespace Wt {

template <class Widget>
class IndexEdit final : public Widget
{
public:
  IndexEdit(const WModelIndex& index)
    : index_(index)
  { }

  void setIndex(const WModelIndex& index) {
    index_ = index;
  }

  const WModelIndex& index() const {
    return index_;
  }

  virtual WString toolTip() const override
  {
    if (index_.flags().test(ItemFlag::DeferredToolTip))
      return asString(index_.data(ItemDataRole::ToolTip));
    else
      return Widget::toolTip();
  }

private:
  WModelIndex index_;
};

#ifdef WT_CNOR
class IndexCheckBox : public IndexEdit<WCheckBox>
{
public:
  IndexCheckBox(const WModelIndex& index);

  void setIndex(const WModelIndex& index);
  const WModelIndex& index();
  virtual WString toolTip() const;
};

class IndexContainerWidget : public IndexEdit<WContainerWidget>
{
public:
  IndexContainerWidget(const WModelIndex& index);

  void setIndex(const WModelIndex& index);
  const WModelIndex& index();
  virtual WString toolTip() const;
};

class IndexAnchor : public IndexEdit<WAnchor>
{
public:
  IndexAnchor(const WModelIndex& index);

  void setIndex(const WModelIndex& index);
  const WModelIndex& index();
  virtual WString toolTip() const;
};

class IndexText : public IndexEdit<WText>
{
public:
  IndexText(const WModelIndex& index);

  void setIndex(const WModelIndex& index);
  const WModelIndex& index();
  virtual WString toolTip() const;
};
#endif // WT_CNOR

WItemDelegate::WItemDelegate()
{ }

void WItemDelegate::setTextFormat(const WT_USTRING& format)
{
  textFormat_ = format;
}

std::unique_ptr<WWidget> WItemDelegate::update(WWidget *widget, const WModelIndex& index,
                                               WFlags<ViewItemRenderFlag> flags)
{
  bool editing = widget && widget->find("t") == nullptr;

  WidgetRef widgetRef(widget);

  if (flags.test(ViewItemRenderFlag::Editing)) {
    if (!editing) {
      widgetRef.created = createEditor(index, flags);
      widgetRef.w = widgetRef.created.get();
      WInteractWidget *iw = dynamic_cast<WInteractWidget *>(widget);
      if (iw) {
        // Disable drag & drop and selection behaviour
        iw->mouseWentDown().preventPropagation();
        iw->clicked().preventPropagation();
      }
    }
  } else {
    if (editing)
      widgetRef.w = nullptr;
  }

  bool isNew = false;

  bool haveCheckBox = index.isValid() ? cpp17::any_has_value(index.data(ItemDataRole::Checked)) : false;
  bool haveLink = index.isValid() ? cpp17::any_has_value(index.data(ItemDataRole::Link)) : false;
  bool haveIcon = index.isValid() ? cpp17::any_has_value(index.data(ItemDataRole::Decoration)): false;
  if (!(flags & ViewItemRenderFlag::Editing)) {
    if (widgetRef.w) {
      if (haveCheckBox != (checkBox(widgetRef, index, false) != 0) ||
          haveLink != (anchorWidget(widgetRef, index, false) != 0) ||
          haveIcon != (iconWidget(widgetRef, index, false) != 0)) {
        widgetRef.w->removeFromParent();
        widgetRef.w = nullptr;
      }
    }

    if (!widgetRef.w) {
      isNew = true;
      widgetRef.created = std::unique_ptr<WWidget>(new IndexText(index));
      IndexText *t = static_cast<IndexText*>(widgetRef.created.get());
      t->setObjectName("t");
      if (index.isValid() && !(index.flags() & ItemFlag::XHTMLText))
        t->setTextFormat(TextFormat::Plain);
      t->setWordWrap(true);
      widgetRef.w = t;
    }

    if (!index.isValid()) {
      if (isNew)
        return std::move(widgetRef.created);
      else
        return nullptr;
    }

    cpp17::any checkedData = index.data(ItemDataRole::Checked);
    if (cpp17::any_has_value(checkedData)) {
      CheckState state =
        (checkedData.type() == typeid(bool) ?
         (cpp17::any_cast<bool>(checkedData) ?
          CheckState::Checked : CheckState::Unchecked)
         : (checkedData.type() == typeid(CheckState) ?
            cpp17::any_cast<CheckState>(checkedData) :
            CheckState::Unchecked));
      IndexCheckBox *icb =
        checkBox(widgetRef, index, true, true, index.flags().test(ItemFlag::Tristate));
      icb->setCheckState(state);
      icb->setEnabled(index.flags().test(ItemFlag::UserCheckable));
    } else if (!isNew) {
      IndexCheckBox *icb =
        checkBox(widgetRef, index, false);
      if (icb)
        icb->removeFromParent();
    }

    cpp17::any linkData = index.data(ItemDataRole::Link);
    if (cpp17::any_has_value(linkData)) {
      WLink link = cpp17::any_cast<WLink>(linkData);
      IndexAnchor *a = anchorWidget(widgetRef, index, true);
      a->setLink(link);
    }

    IndexText *t = textWidget(widgetRef, index);

    WString label = asString(index.data(), textFormat_);
    if (label.empty() && haveCheckBox)
      label = WString::fromUTF8(" ");
    t->setText(label);

    std::string iconUrl = asString(index.data(ItemDataRole::Decoration)).toUTF8();
    if (!iconUrl.empty()) {
      iconWidget(widgetRef, index, true)->setImageLink(WLink(iconUrl));
    } else if (!isNew) {
      auto icw = iconWidget(widgetRef, index, false);
      if (icw)
        icw->removeFromParent();
    }
  }

  if (index.flags().test(ItemFlag::DeferredToolTip)) {
    widgetRef.w->setDeferredToolTip(true,
                                    index.flags().test(ItemFlag::XHTMLText) ?
                                    TextFormat::XHTML :
                                    TextFormat::Plain);
  } else {
    WString tooltip = asString(index.data(ItemDataRole::ToolTip));
    if (!tooltip.empty() || !isNew)
      widgetRef.w->setToolTip(tooltip,
                              index.flags().test(ItemFlag::XHTMLText) ?
                              TextFormat::XHTML : TextFormat::Plain);
  }

  WT_USTRING sc = asString(index.data(ItemDataRole::StyleClass));

  if (flags.test(ViewItemRenderFlag::Selected))
    sc += WT_USTRING::fromUTF8
      (" " + WApplication::instance()->theme()->activeClass());

  if (flags.test(ViewItemRenderFlag::Editing))
    sc += WT_USTRING::fromUTF8(" Wt-delegate-edit");

  widgetRef.w->setStyleClass(sc);

  if (index.flags().test(ItemFlag::DropEnabled))
    widgetRef.w->setAttributeValue("drop", WString::fromUTF8("true"));
  else
    if (!widgetRef.w->attributeValue("drop").empty())
      widgetRef.w->setAttributeValue("drop", WString::fromUTF8("f"));

  return std::move(widgetRef.created);
}

/*
 * Possible layouts:
 *  1) WText "t"
 * or
 *  2) WContainerWidget "o" ([WCheckbox "c"] [WImage "i"] [inv] WText "t")
 * or
 *  3) WAnchor "a" ([WImage "i"] [inv] WText "t")
 * or
 *  4) WContainerWidget "o" ([WCheckbox "c"] WAnchor "a" ([Image "i"] [inv] WText "t"))
 */

IndexCheckBox *WItemDelegate::checkBox(WidgetRef& w, const WModelIndex& index,
                                      bool autoCreate, bool update, bool triState)
{
  IndexCheckBox *checkBox = dynamic_cast<IndexCheckBox *>(w.w->find("c"));

  if (!checkBox) {
    if (autoCreate) {
      std::unique_ptr<IndexCheckBox> newBox
        (checkBox = new IndexCheckBox(index));
      checkBox->setObjectName("c");
      checkBox->clicked().preventPropagation();

      IndexContainerWidget *wc =
          dynamic_cast<IndexContainerWidget *>(w.w->find("o"));
      if (!wc) {
        std::unique_ptr<WWidget> oldW;
        if (w.created)
          oldW = std::move(w.created);
        w.created = std::unique_ptr<WWidget>(new IndexContainerWidget(index));
        wc = static_cast<IndexContainerWidget*>(w.created.get());
        wc->setObjectName("o");
        w.w->setInline(true);
        w.w->setStyleClass(WString::Empty);

        /* We first remove to avoid reparenting warnings */
        if (w.w->parent()) {
          assert(!oldW);
          oldW = w.w->removeFromParent();
        }

        wc->addWidget(std::move(oldW));
        w.w = wc;
      }

      wc->insertWidget(0, std::move(newBox));
      IndexCheckBox *const cb = checkBox;
      checkBox->changed().connect
        (this, std::bind(&WItemDelegate::onCheckedChange, this, cb));
    } else
      return nullptr;
  }

  if (update)
    checkBox->setTristate(triState);

  return checkBox;
}

IndexText *WItemDelegate::textWidget(WidgetRef& w, const WModelIndex &index)
{
  return dynamic_cast<IndexText *>(w.w->find("t"));
}

WImage *WItemDelegate::iconWidget(WidgetRef& w,
                                  const WModelIndex& index, bool autoCreate)
{
  WImage *image = dynamic_cast<WImage *>(w.w->find("i"));
  if (image || !autoCreate)
    return image;

  WContainerWidget *wc =
      dynamic_cast<IndexAnchor *>(w.w->find("a"));

  if (!wc)
    wc = dynamic_cast<IndexContainerWidget *>(w.w->find("o"));

  if (!wc) {
    std::unique_ptr<WWidget> newWc(new IndexContainerWidget(index));
    wc = static_cast<IndexContainerWidget*>(newWc.get());
    wc->setObjectName("o");
    wc->addWidget(w.created ? std::move(w.created) : w.w->removeFromParent());
    w.created = std::move(newWc);
    w.w = wc;
  }

  std::unique_ptr<WWidget> newImage(new WImage());
  image = static_cast<WImage*>(newImage.get());
  image->setObjectName("i");
  image->setStyleClass("icon");
  wc->insertWidget(wc->count() - 1, std::move(newImage));

  // IE does not want to center vertically without this:
  if (wApp->environment().agentIsIE()) {
    std::unique_ptr<WImage>inv(new WImage(wApp->onePixelGifUrl()));
    inv->setStyleClass("rh w0 icon");
    inv->resize(0, WLength::Auto);
    wc->insertWidget(wc->count() -1, std::move(inv));
  }

  return image;
}

IndexAnchor *WItemDelegate::anchorWidget(WidgetRef& w, const WModelIndex &index, bool autoCreate)
{
  IndexAnchor *anchor = dynamic_cast<IndexAnchor *>(w.w->find("a"));
  if (anchor || !autoCreate)
    return anchor;

  std::unique_ptr<WWidget> newAnchor(new IndexAnchor(index));
  anchor = static_cast<IndexAnchor*>(newAnchor.get());
  anchor->setObjectName("a");

  IndexContainerWidget *wc =
    dynamic_cast<IndexContainerWidget *>(w.w->find("o"));
  if (wc) {
    /*
     * Convert (2) -> (4)
     */
    int firstToMove = 0;

    WCheckBox *cb = dynamic_cast<WCheckBox *>(wc->widget(0));
    if (cb)
      firstToMove = 1;

    wc->insertWidget(firstToMove, std::move(newAnchor));

    while (wc->count() > firstToMove + 1) {
      WWidget *c = wc->widget(firstToMove + 1);
      auto uc = wc->removeWidget(c);
      anchor->addWidget(std::move(uc));
    }
  } else {
    /*
     * Convert (1) -> (3)
     */
    anchor->addWidget(w.created ? std::move(w.created) : w.w->removeFromParent());
    w.created = std::move(newAnchor);
    w.w = anchor;
  }

  return anchor;
}

void WItemDelegate::updateModelIndex(WWidget *widget, const WModelIndex& index)
{
  WidgetRef w(widget);

  if (index.flags().test(ItemFlag::UserCheckable)) {
    IndexCheckBox *cb = checkBox(w, index, false);
    if (cb)
      cb->setIndex(index);
  }

  if (index.flags().test(ItemFlag::DeferredToolTip)) {
    IndexText *text = dynamic_cast<IndexText *>(widget);
    if (text)
      text->setIndex(index);

    IndexAnchor *anchor = dynamic_cast<IndexAnchor *>(widget);
    if (anchor)
      anchor->setIndex(index);

    IndexContainerWidget *c = dynamic_cast<IndexContainerWidget *>(widget);
    if (c)
      c->setIndex(index);
  }
}

void WItemDelegate::onCheckedChange(IndexCheckBox *cb) const
{
  WAbstractItemModel *model
    = const_cast<WAbstractItemModel *>(cb->index().model());

  if (cb->isTristate())
    model->setData(cb->index(), cpp17::any(cb->checkState()),
                   ItemDataRole::Checked);
  else
    model->setData(cb->index(), cpp17::any(cb->isChecked()),
                   ItemDataRole::Checked);
}

std::unique_ptr<WWidget> WItemDelegate
::createEditor(const WModelIndex& index,
               WFlags<ViewItemRenderFlag> flags) const
{
  std::unique_ptr<IndexContainerWidget> result(new IndexContainerWidget(index));
  result->setSelectable(true);

  std::unique_ptr<WLineEdit> lineEdit(new WLineEdit());
  lineEdit->setText(asString(index.data(ItemDataRole::Edit), textFormat_));
  IndexContainerWidget *const resultPtr = result.get();
  lineEdit->enterPressed().connect
    (this, std::bind(&WItemDelegate::doCloseEditor, this, resultPtr, true));
  lineEdit->escapePressed().connect
    (this, std::bind(&WItemDelegate::doCloseEditor, this, resultPtr, false));
  lineEdit->escapePressed().preventPropagation();

  if (flags.test(ViewItemRenderFlag::Focused))
    lineEdit->setFocus(true);

  lineEdit->resize(WLength(100, LengthUnit::Percentage),
                     WLength(100, LengthUnit::Percentage)); //for Konqueror
  result->addWidget(std::move(lineEdit));

  return std::move(result);
}

void WItemDelegate::doCloseEditor(WWidget *editor, bool save) const
{
  closeEditor().emit(editor, save);
}

cpp17::any WItemDelegate::editState(WWidget *editor, const WModelIndex& index)
  const
{
  IndexContainerWidget *w =
      dynamic_cast<IndexContainerWidget *>(editor);
  WLineEdit *lineEdit = dynamic_cast<WLineEdit *>(w->widget(0));

  return cpp17::any(lineEdit->text());
}

void WItemDelegate::setEditState(WWidget *editor, const WModelIndex& index,
                                 const cpp17::any& value) const
{
  IndexContainerWidget *w =
      dynamic_cast<IndexContainerWidget *>(editor);
  WLineEdit *lineEdit = dynamic_cast<WLineEdit *>(w->widget(0));

  lineEdit->setText(cpp17::any_cast<WT_USTRING>(value));
}

void WItemDelegate::setModelData(const cpp17::any& editState,
                                 WAbstractItemModel *model,
                                 const WModelIndex& index) const
{
  model->setData(index, editState, ItemDataRole::Edit);
}

}
