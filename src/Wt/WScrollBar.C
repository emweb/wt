/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WScrollBar"
#include "Wt/WScrollArea"

#include "DomElement.h"
#include "WebUtils.h"

namespace Wt {

WScrollBar::WScrollBar(WScrollArea *area, Orientation orientation)
  : scrollArea_(area),
    orientation_(orientation),
    tiesChanged_(false),
    valueSet_(false)
{ }

WScrollBar::~WScrollBar()
{
  while (ties_.size())
    unTie(this, ties_[0]);
}

void WScrollBar::setValue(int value)
{
  value_ = value;
  valueSet_ = true;

  scrollArea_->scrollBarChanged();
}

void WScrollBar::tie(WScrollBar *one, WScrollBar *two)
{
  one->ties_.push_back(two);
  two->ties_.push_back(one);

  one->tiesChanged_ = true;
  two->tiesChanged_ = true;
  one->scrollArea_->scrollBarChanged();
  two->scrollArea_->scrollBarChanged();
}

void WScrollBar::unTie(WScrollBar *one, WScrollBar *two)
{
  Utils::erase(one->ties_, two);
  Utils::erase(two->ties_, one);

  one->tiesChanged_ = true;
  two->tiesChanged_ = true;
  one->scrollArea_->scrollBarChanged();
  two->scrollArea_->scrollBarChanged();
}

void WScrollBar::updateDom(DomElement& element, bool all)
{
  if (valueSet_) {
    std::string side = (orientation_ == Horizontal ? "Left" : "Top");
    
    element.callMethod("scroll" + side + " = " +
		       boost::lexical_cast<std::string>(value_) + ";"); 

    valueSet_ = false;
  }

  if (tiesChanged_ || all) {
    std::string jsCode;
    for (unsigned i = 0; i < ties_.size(); ++i) {
      std::string tieElement
	= WT_CLASS ".getElement('" + ties_[i]->scrollArea_->id() + "')";
      std::string scrollElement
	= WT_CLASS ".getElement('" + scrollArea_->id() + "')";
      std::string side = (orientation_ == Horizontal ? "Left" : "Top");

      jsCode +=	tieElement + ".scroll" + side
	+ "=" + scrollElement + ".scroll" + side + ";";
    }

    if (!all || !jsCode.empty())
      element.addEvent("scroll", jsCode);

    tiesChanged_ = false;
  }
}

/*
  for iframe (but does not work either in konqueror)

  window.pageXOffset
  window.pageYOffset
  document.body.scrollLeft
  document.body.scrollTop

  window.scrollTo()
*/

}
