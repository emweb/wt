/*
 * Copyright (C) 2009 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WItemDelegate"

#include "Wt/WAbstractItemModel"
#include "Wt/WAnchor"
#include "Wt/WApplication"
#include "Wt/WCheckBox"
#include "Wt/WContainerWidget"
#include "Wt/WEnvironment"
#include "Wt/WImage"
#include "Wt/WModelIndex"
#include "Wt/WText"

#include "Utils.h"

namespace Wt {

class ItemCheckBox : public WCheckBox
{
public:
  ItemCheckBox(const WModelIndex& index)
    : WCheckBox(),
      index_(index)
  { }

  void setIndex(const WModelIndex& index) {
    index_ = index;
  }
  
  const WModelIndex& index() const { return index_; }

private:
  WModelIndex index_;
};

WItemDelegate::WItemDelegate(WObject *parent)
  : WAbstractItemDelegate(parent)
{
  checkedChangeMapper_ = new WSignalMapper<ItemCheckBox *>(this);
  checkedChangeMapper_->mapped().connect
    (SLOT(this, WItemDelegate::onCheckedChange));
}

void WItemDelegate::setTextFormat(const WT_USTRING& format)
{
  textFormat_ = format;
}

WWidget *WItemDelegate::update(WWidget *widget, const WModelIndex& index,
			       WFlags<ViewItemRenderFlag> flags)
{
  WidgetRef widgetRef(widget);

  bool isNew = false;

  if (!widgetRef.w) {
    isNew = true;
    WText *t = new WText();
    if (index.isValid() && !(index.flags() & ItemIsXHTMLText))
      t->setTextFormat(PlainText);
    t->setWordWrap(true);
    widgetRef.w = t;
  }

  if (!index.isValid())
    return widgetRef.w;

  bool haveCheckBox = false;

  if (index.flags() & ItemIsUserCheckable) {
    boost::any checkedData = index.data(CheckStateRole);
    CheckState state = checkedData.empty() ? Unchecked
      : (checkedData.type() == typeid(bool) ?
	 (boost::any_cast<bool>(checkedData) ? Checked : Unchecked)
	 : (checkedData.type() == typeid(CheckState) ?
	    boost::any_cast<CheckState>(checkedData) : Unchecked));
    checkBox(widgetRef, index, true, index.flags() & ItemIsTristate)
      ->setCheckState(state);
    haveCheckBox = true;
  } else if (!isNew)
      delete checkBox(widgetRef, index, false);

  std::string internalPath = asString(index.data(InternalPathRole)).toUTF8();
  std::string url = asString(index.data(UrlRole)).toUTF8();

  if (!internalPath.empty() || !url.empty()) {
    WAnchor *a = anchorWidget(widgetRef);

    if (!internalPath.empty())
      a->setRefInternalPath(internalPath);
    else
      a->setRef(url);
  }

  WText *t = textWidget(widgetRef);

  WString label = asString(index.data(), textFormat_);
  if (label.empty() && haveCheckBox)
    label = WString::fromUTF8(" ");
  t->setText(label);

  std::string iconUrl = asString(index.data(DecorationRole)).toUTF8();
  if (!iconUrl.empty()) {
    iconWidget(widgetRef, true)->setImageRef(iconUrl);
  } else if (!isNew)
      delete iconWidget(widgetRef, false);

  WString tooltip = asString(index.data(ToolTipRole));
  if (!tooltip.empty() || !isNew)
    widgetRef.w->setToolTip(tooltip);

  if (index.column() != 0) {
    WT_USTRING sc = asString(index.data(StyleClassRole));

    if (flags & RenderSelected)
      sc += WT_USTRING::fromUTF8(" Wt-selected");

    widgetRef.w->setStyleClass(sc);
  } else
    widgetRef.w->setStyleClass(WString::Empty);

  if (index.flags() & ItemIsDropEnabled)
    widgetRef.w->setAttributeValue("drop", WString::fromUTF8("true"));
  else
    if (!widgetRef.w->attributeValue("drop").empty())
      widgetRef.w->setAttributeValue("drop", WString::fromUTF8("f"));

  return widgetRef.w;
}

ItemCheckBox *WItemDelegate::checkBox(WidgetRef& w, const WModelIndex& index,
				      bool autoCreate, bool triState)
{
  WText *t = dynamic_cast<WText *>(w.w);
  WAnchor *a = dynamic_cast<WAnchor *>(w.w);
  WContainerWidget *wc = dynamic_cast<WContainerWidget *>(w.w);

  /*
   * Case 1 or 3
   */
  if (t || a)
    if (autoCreate) {
      wc = new WContainerWidget();
      wc->addWidget(w.w);
      w.w = wc;
    } else
      return 0;

  ItemCheckBox *cb = dynamic_cast<ItemCheckBox *>(wc->widget(0));

  if (!cb && autoCreate) {
    cb = new ItemCheckBox(index);
    wc->insertWidget(0, cb);
    checkedChangeMapper_->mapConnect(cb->changed(), cb);
  }

  if (cb)
    cb->setTristate(triState);

  return cb;
}

/*
 * Possible layouts:
 *  1) WText
 * or
 *  2) WContainerWidget ([WCheckbox] [WImage] [inv] WText)
 * or
 *  3) WAnchor ([WImage] [inv] WText)
 * or
 *  4) WContainerWidget ([WCheckbox] WAnchor ([Image] [inv] WText))
 */
WText *WItemDelegate::textWidget(WidgetRef& w)
{
  /*
   * Case 1
   */
  WText *result = dynamic_cast<WText *>(w.w);
  if (result)
    return result;

  /* Cases 2-3 */
  WContainerWidget *wc = dynamic_cast<WContainerWidget *>(w.w);

  result = dynamic_cast<WText *>(wc->widget(wc->count() - 1));
  if (result)
    return result;

  /* Case 4 */
  wc = dynamic_cast<WContainerWidget *>(wc->widget(wc->count() - 1));

  return dynamic_cast<WText *>(wc->widget(wc->count() - 1));
}

WImage *WItemDelegate::iconWidget(WidgetRef& w, bool autoCreate)
{
  /*
   * Case 1
   */
  WText *result = dynamic_cast<WText *>(w.w);
  if (result)
    if (autoCreate) {
      WContainerWidget *wc = new WContainerWidget();

      WImage *image = new WImage();
      image->setStyleClass("icon");
      wc->addWidget(image);

      // IE does not want to center vertically without this:
      if (wApp->environment().agentIsIE()) {
	WImage *inv = new WImage(wApp->onePixelGifUrl());
	inv->setStyleClass("rh w0 icon");
	inv->resize(0, WLength::Auto);
	wc->addWidget(inv);
      }

      wc->addWidget(w.w);
      w.w = wc;

      return image;
    } else
      return 0;

  /* Cases 2-4 */
  WContainerWidget *wc = dynamic_cast<WContainerWidget *>(w.w);

  for (int i = 0; i < wc->count(); ++i) {
    WImage *image = dynamic_cast<WImage *>(wc->widget(i));
    if (image)
      return image;

    WAnchor *anchor = dynamic_cast<WAnchor *>(wc->widget(i));
    if (anchor) {
      wc = anchor;
      i = -1;
    }
  }

  if (autoCreate) {
    WImage *image = new WImage();
    image->setStyleClass("icon");
    wc->insertWidget(wc->count() - 1, image);

    // IE does not want to center vertically without this:
    if (wApp->environment().agentIsIE()) {
      WImage *inv = new WImage(wApp->onePixelGifUrl());
      inv->setStyleClass("rh w0 icon");
      wc->insertWidget(wc->count() - 1, inv);
    }

    return image;
  } else
    return 0;
}

WAnchor *WItemDelegate::anchorWidget(WidgetRef& w)
{
  /*
   * Case 3
   */
  WAnchor *result = dynamic_cast<WAnchor *>(w.w);
  if (result)
    return result;
  else {
    /*
     * Case 1
     */
    WText *text = dynamic_cast<WText *>(w.w);
    if (text) {
      WAnchor *a = new WAnchor();
      a->addWidget(w.w);
      w.w = a;

      return a;
    }

    /*
     * Case 4
     */
    WContainerWidget *wc = dynamic_cast<WContainerWidget *>(w.w);
    WWidget *lw = wc->widget(wc->count() - 1);

    WAnchor *a = dynamic_cast<WAnchor *>(lw);
    if (a)
      return a;

    /*
     * Case 2
     */
    a = new WAnchor();
    int firstToMove = 0;

    WCheckBox *cb = dynamic_cast<WCheckBox *>(wc->widget(0));
    if (cb)
      firstToMove = 1;

    wc->insertWidget(firstToMove, a);

    while (wc->count() > firstToMove + 1) { 
      WWidget *c = wc->widget(firstToMove + 1);
      wc->removeWidget(c);
      a->addWidget(c);
    }

    return a;
  }
}

void WItemDelegate::updateModelIndex(WWidget *widget, const WModelIndex& index)
{
  WidgetRef w(widget);
  if (index.flags() & ItemIsUserCheckable) {
    ItemCheckBox *cb = checkBox(w, index, false, false);
    if (cb)
      cb->setIndex(index);
  }
}

void WItemDelegate::onCheckedChange(ItemCheckBox *cb)
{
  WAbstractItemModel *model
    = const_cast<WAbstractItemModel *>(cb->index().model());

  if (cb->isTristate())
    model->setData(cb->index(), boost::any(cb->checkState()), CheckStateRole);
  else
    model->setData(cb->index(), boost::any(cb->isChecked()), CheckStateRole);
}

}
