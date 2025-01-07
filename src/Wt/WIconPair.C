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
  : clickIsSwitch_(clickIsSwitch),
    impl_(new WContainerWidget())
{
  iconStr_[0] = icon1Str;
  iconStr_[1] = icon2Str;
  wicon_[0] = nullptr;
  wicon_[1] = nullptr;
  setImplementation(std::unique_ptr<WWidget>(impl_));
  image_[0] = impl_->addWidget(std::make_unique<WImage>(WLink(icon1Str)));
  image_[1] = impl_->addWidget(std::make_unique<WImage>(WLink(icon2Str)));
  impl_->setLoadLaterWhenInvisible(false);

  setInline(true);

  image_[1]->hide();

  if (clickIsSwitch) {
#ifndef WT_TARGET_JAVA
    std::string fic1 = image_[0]->id();
    std::string fic2 = image_[1]->id();
    std::string hide_1 = WT_CLASS ".hide('" + fic1 +"');";
    std::string show_1 = WT_CLASS ".inline('" + fic1 +"');";
    std::string hide_2 = WT_CLASS ".hide('" + fic2 +"');";
    std::string show_2 = WT_CLASS ".inline('" + fic2 +"');";
    implementJavaScript(&WIconPair::showIcon1, hide_2 + show_1
                        + WT_CLASS ".cancelEvent(e);");
    implementJavaScript(&WIconPair::showIcon2, hide_1 + show_2
                        + WT_CLASS ".cancelEvent(e);");
#else
    image_[0]->clicked().preventPropagation();
    image_[1]->clicked().preventPropagation();
#endif // WT_TARGET_JAVA

    image_[0]->clicked().connect(this, &WIconPair::showIcon2);
    image_[1]->clicked().connect(this, &WIconPair::showIcon1);

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
  resetIcon(0, type);
  if (clickIsSwitch_) {
    usedIcon1()->clicked().connect(this, &WIconPair::showIcon2);
  }
}

void WIconPair::setIcon2Type(IconType type)
{
  resetIcon(1, type);
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

WInteractWidget *WIconPair::usedIcon(int i) const
{
  if (wicon_[i]) {
    return wicon_[i];
  }
  return image_[i];
}

void WIconPair::resetIcon(int i, IconType type)
{
  int currentState = state();
  if (wicon_[i]) {
    impl_->removeWidget(wicon_[i]);
  }
  if (image_[i]) {
    impl_->removeWidget(image_[i]);
  }
  if (type == IconType::IconName) {
    wicon_[i] = impl_->addWidget(std::make_unique<WIcon>(iconStr_[i]));
    image_[i] = nullptr;
  } else {
    image_[i] = impl_->addWidget(std::make_unique<WImage>(WLink(iconStr_[i])));
    wicon_[i] = nullptr;
  }
  resetLearnedSlots();
  setState(currentState);
}

}
