/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WIcon.h"
#include "Wt/WImage.h"
#include "Wt/WContainerWidget.h"
#include "Wt/WCssDecorationStyle.h"
#include "Wt/WIconPair.h"

namespace Wt {

WIconPair::WIconPair(const std::string& icon1Str, const std::string& icon2Str,
                     bool clickIsSwitch)
  : icon1Str_(icon1Str),
    icon2Str_(icon2Str),
    clickIsSwitch_(clickIsSwitch),
    impl_(new WContainerWidget()),
    wicon1_(nullptr),
    wicon2_(nullptr)
{
  setImplementation(std::unique_ptr<WWidget>(impl_));
  image1_ = impl_->addWidget(std::make_unique<WImage>(icon1Str));
  image2_ = impl_->addWidget(std::make_unique<WImage>(icon2Str));
  impl_->setLoadLaterWhenInvisible(false);

  setInline(true);

  image2_->hide();

  if (clickIsSwitch) {
#ifndef WT_TARGET_JAVA
    std::string fic1 = image1_->id();
    std::string fic2 = image2_->id();
    std::string hide_1 = WT_CLASS ".hide('" + fic1 +"');";
    std::string show_1 = WT_CLASS ".inline('" + fic1 +"');";
    std::string hide_2 = WT_CLASS ".hide('" + fic2 +"');";
    std::string show_2 = WT_CLASS ".inline('" + fic2 +"');";
    implementJavaScript(&WIconPair::showIcon1, hide_2 + show_1
                        + WT_CLASS ".cancelEvent(e);");
    implementJavaScript(&WIconPair::showIcon2, hide_1 + show_2
                        + WT_CLASS ".cancelEvent(e);");
#else
    image1_->clicked().preventPropagation();
    image2_->clicked().preventPropagation();
#endif // WT_TARGET_JAVA

    image1_->clicked().connect(this, &WIconPair::showIcon2);
    image2_->clicked().connect(this, &WIconPair::showIcon1);

    decorationStyle().setCursor(Cursor::PointingHand);
  }
}
void WIconPair::setIconsType(IconType type)
{
  setIcon1Type(type);
  setIcon2Type(type);
}

void WIconPair::setIcon1Type(IconType type)
{
  resetIcon(image1_, wicon1_, icon1Str_, type);
  if (clickIsSwitch_) {
    usedIcon1()->clicked().connect(this, &WIconPair::showIcon2);
  }
}

void WIconPair::setIcon2Type(IconType type)
{
  resetIcon(image2_, wicon2_, icon2Str_, type);
  if (clickIsSwitch_) {
    usedIcon2()->clicked().connect(this, &WIconPair::showIcon1);
  }
}

void WIconPair::setState(int num)
{
  if (num == 0) {
    usedIcon1()->show();
    usedIcon2()->hide();
  } else {
    usedIcon1()->hide();
    usedIcon2()->show();
  }
}

int WIconPair::state() const
{
  return (usedIcon1()->isHidden() ? 1 : 0);
}

void WIconPair::showIcon1()
{
  setState(0);
}

void WIconPair::showIcon2()
{
  setState(1);
}

EventSignal<WMouseEvent>& WIconPair::icon1Clicked()
{
  return usedIcon1()->clicked();
}

EventSignal<WMouseEvent>& WIconPair::icon2Clicked()
{
  return usedIcon2()->clicked();
}

WInteractWidget *WIconPair::usedIcon1() const
{
  if (wicon1_) {
    return wicon1_;
  }
  return image1_;
}

WInteractWidget *WIconPair::usedIcon2() const
{
  if (wicon2_) {
    return wicon2_;
  }
  return image2_;
}

void WIconPair::resetIcon(WImage *&image, WIcon *&wicon, const std::string &iconStr, IconType type)
{
  int currentState = state();
  if (wicon) {
    impl_->removeWidget(wicon);
  }
  if (image) {
    impl_->removeWidget(image);
  }
  if (type == IconType::IconName) {
    wicon = impl_->addWidget(std::make_unique<WIcon>(iconStr));
    image = nullptr;
  } else {
    image = impl_->addWidget(std::make_unique<WImage>(iconStr));
    wicon = nullptr;
  }
  resetLearnedSlots();
  setState(currentState);
}

}
