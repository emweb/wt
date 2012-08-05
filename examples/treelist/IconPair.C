/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <Wt/WCssDecorationStyle>
#include <Wt/WContainerWidget>
#include <Wt/WImage>

#include "IconPair.h"

IconPair::IconPair(const std::string icon1URI, const std::string icon2URI,
		   bool clickIsSwitch, Wt::WContainerWidget *parent)
  : Wt::WCompositeWidget(parent),
    impl_(new Wt::WContainerWidget()),
    icon1_(new Wt::WImage(icon1URI, impl_)),
    icon2_(new Wt::WImage(icon2URI, impl_)),
    icon1Clicked(icon1_->clicked()),
    icon2Clicked(icon2_->clicked())
{
  setImplementation(impl_);

  implementStateless(&IconPair::showIcon1, &IconPair::undoShowIcon1);
  implementStateless(&IconPair::showIcon2, &IconPair::undoShowIcon2);

  setInline(true);

  icon2_->hide();

  if (clickIsSwitch) {
    icon1_->clicked().connect(icon1_, &Wt::WImage::hide);
    icon1_->clicked().connect(icon2_, &Wt::WImage::show);

    icon2_->clicked().connect(icon2_, &Wt::WImage::hide);
    icon2_->clicked().connect(icon1_, &Wt::WImage::show); //

    decorationStyle().setCursor(Wt::PointingHandCursor);
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
