// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WWIDGET_ITEM_H_
#define WWIDGET_ITEM_H_

#include <Wt/WLayoutItem.h>
#include <Wt/WWidgetItemImpl.h>

#include <memory>

namespace Wt {

class WWidgetItemImpl;

/*! \class WWidgetItem Wt/WWidgetItem.h Wt/WWidgetItem.h
 *  \brief A layout item that holds a single widget.
 *
 * \sa WLayout::addWidget(WWidget *)
 */
class WT_API WWidgetItem : public WLayoutItem
{
public:
  /*! \brief Creates a new item for the given <i>widget</i>.
   */
  WWidgetItem(std::unique_ptr<WWidget> widget);

  virtual ~WWidgetItem();

  virtual WWidget *widget() override { return widget_.get(); }
  virtual WLayout *layout() override { return nullptr; }
  virtual WLayout *parentLayout() const override { return parentLayout_; }
  virtual WWidget *parentWidget() const override;

  virtual WWidgetItem *findWidgetItem(WWidget *widget) override;

  virtual WWidgetItemImpl *impl() const override { return impl_.get(); }

  std::unique_ptr<WWidget> takeWidget();

  virtual void iterateWidgets(const HandleWidgetMethod& method) const override;

private:
  std::unique_ptr<WWidget> widget_;
  WLayout *parentLayout_;
  std::unique_ptr<WWidgetItemImpl> impl_;

  virtual void setParentWidget(WWidget *parent) override;
  virtual void setParentLayout(WLayout *layout) override;
};

}

#endif // WWIDGET_ITEM_H_
