// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WFIT_LAYOUT_H_
#define WFIT_LAYOUT_H_

#include <Wt/WLayout.h>
#include <Wt/WGridLayout.h>

namespace Wt {

/*! \class WFitLayout Wt/WFitLayout.h Wt/WFitLayout.h
 *  \brief A layout manager which spans a single widget to all available space.
 *
 * This layout manager may manage only a single child widget, and sizes that
 * widget so that it uses all space available in the parent.
 */
class WT_API WFitLayout : public WLayout
{
public:
  /*! \brief Creates a new fit layout.
   */
  WFitLayout();
  ~WFitLayout();

  static void fitWidget(WContainerWidget *container,
			std::unique_ptr<WWidget> widget);

  virtual void addItem(std::unique_ptr<WLayoutItem> item) override;
  virtual std::unique_ptr<WLayoutItem> removeItem(WLayoutItem *item) override;
  virtual WLayoutItem *itemAt(int index) const override;
  virtual int indexOf(WLayoutItem *item) const override;
  virtual int count() const override;

protected:
  virtual void iterateWidgets(const HandleWidgetMethod& method) const override;
  virtual void updateImplementation() override;

private:
  Impl::Grid grid_;

  virtual void setParentWidget(WWidget *parent) override;
};

}

#endif // WFIT_LAYOUT_H_
