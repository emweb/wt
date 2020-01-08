/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <Wt/WCssDecorationStyle.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WImage.h>

#include "IconPair.h"

using namespace Wt;

IconPair::IconPair(const std::string icon1URI, const std::string icon2URI,
                   bool clickIsSwitch)
  : WCompositeWidget(),
    impl_(nullptr),
    icon1_(nullptr),
    icon2_(nullptr),
    icon1Clicked(nullptr),
    icon2Clicked(nullptr)
{
  auto impl = cpp14::make_unique<WContainerWidget>();
  impl_ = impl.get();
  icon1_ = impl_->addWidget(cpp14::make_unique<WImage>(icon1URI));
  icon2_ = impl_->addWidget(cpp14::make_unique<WImage>(icon2URI));
  icon1Clicked = &icon1_->clicked();
  icon2Clicked = &icon2_->clicked();

  setImplementation(std::move(impl));

  implementStateless(&IconPair::showIcon1, &IconPair::undoShowIcon1);
  implementStateless(&IconPair::showIcon2, &IconPair::undoShowIcon2);

  setInline(true);

  icon2_->hide();

  if (clickIsSwitch) {
    icon1_->clicked().connect(icon1_, &WImage::hide);
    icon1_->clicked().connect(icon2_, &WImage::show);

    icon2_->clicked().connect(icon2_, &WImage::hide);
    icon2_->clicked().connect(icon1_, &WImage::show); //

    decorationStyle().setCursor(Cursor::PointingHand);
  }
} //

void IconPair::setState(int num)
{
  if (num == 0) {
    icon1_->show();
    icon2_->hide();
  } else {
    icon1_->hide();
    icon2_->show();
  }
}

int IconPair::state() const
{
  return (icon1_->isHidden() ? 1 : 0);
}

void IconPair::showIcon1()
{
  previousState_ = (icon1_->isHidden() ? 1 : 0);
  setState(0);
}

void IconPair::showIcon2()
{
  previousState_ = (icon1_->isHidden() ? 1 : 0);
  setState(1);
}

void IconPair::undoShowIcon1()
{
  setState(previousState_);
}

void IconPair::undoShowIcon2()
{
  setState(previousState_);
} //
