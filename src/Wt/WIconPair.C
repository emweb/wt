/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WImage"
#include "Wt/WContainerWidget"
#include "Wt/WCssDecorationStyle"
#include "Wt/WIconPair"

namespace Wt {

WIconPair::WIconPair(const std::string& icon1URI, const std::string& icon2URI,
		     bool clickIsSwitch, WContainerWidget *parent)
  : WCompositeWidget(parent),
    impl_(new WContainerWidget()),
    icon1_(new WImage(icon1URI, impl_)),
    icon2_(new WImage(icon2URI, impl_))
{
  setImplementation(impl_);
  impl_->setLoadLaterWhenInvisible(false);

  setInline(true);

  icon2_->hide();

  if (clickIsSwitch) {
#ifndef WT_TARGET_JAVA
    std::string fic1 = icon1_->id();
    std::string fic2 = icon2_->id();
    std::string hide_1 = WT_CLASS ".hide('" + fic1 +"');";
    std::string show_1 = WT_CLASS ".inline('" + fic1 +"');";
    std::string hide_2 = WT_CLASS ".hide('" + fic2 +"');";
    std::string show_2 = WT_CLASS ".inline('" + fic2 +"');";
    implementJavaScript(&WIconPair::showIcon1, hide_2 + show_1
			+ WT_CLASS ".cancelEvent(e);");
    implementJavaScript(&WIconPair::showIcon2, hide_1 + show_2
			+ WT_CLASS ".cancelEvent(e);");
#else
    icon1_->clicked().preventPropagation();
    icon2_->clicked().preventPropagation();
#endif // WT_TARGET_JAVA

    icon1_->clicked().connect(this, &WIconPair::showIcon2);
    icon2_->clicked().connect(this, &WIconPair::showIcon1);

    decorationStyle().setCursor(PointingHandCursor);
  }
}

void WIconPair::setState(int num)
{
  if (num == 0) {
    icon1_->show();
    icon2_->hide();
  } else {
    icon1_->hide();
    icon2_->show();
  }
}

int WIconPair::state() const
{
  return (icon1_->isHidden() ? 1 : 0);
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
  return icon1_->clicked();
}

EventSignal<WMouseEvent>& WIconPair::icon2Clicked()
{
  return icon2_->clicked();
}

}
