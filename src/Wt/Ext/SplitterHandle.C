/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/lexical_cast.hpp>

#include "Wt/Ext/SplitterHandle"
#include "Wt/Ext/Splitter"

#include "Wt/WApplication"
#include "DomElement.h"

namespace Wt {
  namespace Ext {

SplitterHandle::SplitterHandle(Splitter *splitter)
  : Widget(),
    splitter_(splitter)
{ 
  WApplication *app = WApplication::instance();

  const char *CSS_RULES_NAME = "Wt::Ext::SplitterHandle";

  if (!app->styleSheet().isDefined(CSS_RULES_NAME)) {
    app->styleSheet().addRule("*.Wt-split-h",
			      "background-color:#C3DAF9;height:100%;"
			      "z-index:3;", CSS_RULES_NAME);
    app->styleSheet().addRule("*.Wt-split-v",
			      "background-color:#C3DAF9;width:100%;"
			      "z-index:3;");
  }

  if (splitter_->orientation() == Horizontal)
    setStyleClass("Wt-split-h");
  else
    setStyleClass("Wt-split-v");

  setPositionScheme(Absolute);
}

std::string SplitterHandle::createJS(DomElement *inContainer)
{
  assert(inContainer);

  WWidget *wb = splitter_->widgetBefore(this);
  WWidget *wa = splitter_->widgetAfter(this);
  SplitterHandle *sb = splitter_->splitterBefore(this);
  SplitterHandle *sa = splitter_->splitterAfter(this);

  std::string result = elVar() + "=new Ext.SplitBar("
    + "'" + inContainer->id() + "',"
    + "'" + wb->id() + "'";

  if (splitter_->orientation() != Horizontal)
    result += ",Ext.SplitBar.VERTICAL";
  result += ");";
  result += elVar()
    + ".setAdapter(new Ext.SplitBar.AbsoluteLayoutAdapter('"
    + parent()->id() + "'));";

  std::string Left, width, Width;

  if (splitter_->orientation() == Horizontal) {
    Left = "Left"; width = "width"; Width = "Width";
  } else {
    Left = "Top"; width = "height"; Width = "Height";
  }

  result += elVar()
    + ".on('moved',function(){"
    "var b=" WT_CLASS ".getElement('" + wb->id() + "');"
    "var a=" WT_CLASS ".getElement('" + wa->id() + "');"
    "var s=" WT_CLASS ".getElement('" + id() + "');"
    "var t=a.offset" + Left + "-(s.offset" + Left + "+s.offset" + Width + ");"
    "a.style.margin" + Left + "=s.offset" + Left + "+s.offset" + Width
    + "+'px';"
    "a.style." + width + "=a.client" + Width + "+t+'px';";

  if (sb) {
    result += sb->adjustMinMaxJS();
  }
  if (sa) {
    result += sa->adjustMinMaxJS();
  }

  result += "});";

  int minSizeB = minSizeBefore().isAuto() 
    ? 0 : (int)minSizeBefore().toPixels();
  int maxSizeB = maxSizeBefore().isAuto() 
    ? 99999 : (int)maxSizeBefore().toPixels();
  int minSizeA = minSizeAfter().isAuto() 
    ? 0 : (int)minSizeAfter().toPixels();
  int maxSizeA = maxSizeAfter().isAuto()
    ? 99999 : (int)maxSizeAfter().toPixels();

  int currentSizeA 
    = (splitter_->orientation() == Horizontal 
       ? (int)wa->width().toPixels() : (int)wa->height().toPixels());
  int currentSizeB
    = (splitter_->orientation() == Horizontal 
       ? (int)wb->width().toPixels() : (int)wb->height().toPixels());

  result += elVar() + ".minSize="
    + boost::lexical_cast<std::string>
    (std::max(minSizeB, currentSizeB + currentSizeA + splitter_->handleWidth()
	      - maxSizeA)) + ";";

  result += elVar() + ".maxSize="
    + boost::lexical_cast<std::string>
    (std::min(maxSizeB, currentSizeB + currentSizeA
	      - minSizeA)) + ";";

  return result;
}

WLength SplitterHandle::minSizeBefore() const
{
  if (splitter_->orientation() == Horizontal)
    return splitter_->widgetBefore(this)->minimumWidth();
  else
    return splitter_->widgetBefore(this)->minimumHeight();
}

WLength SplitterHandle::maxSizeBefore() const
{
  if (splitter_->orientation() == Horizontal)
    return splitter_->widgetBefore(this)->maximumWidth();
  else
    return splitter_->widgetBefore(this)->maximumHeight();
}

WLength SplitterHandle::minSizeAfter() const
{
  if (splitter_->orientation() == Horizontal)
    return splitter_->widgetAfter(this)->minimumWidth();
  else
    return splitter_->widgetAfter(this)->minimumHeight();
}

WLength SplitterHandle::maxSizeAfter() const
{
  if (splitter_->orientation() == Horizontal)
    return splitter_->widgetAfter(this)->maximumWidth();
  else
    return splitter_->widgetAfter(this)->maximumHeight();
}

std::string SplitterHandle::adjustMinMaxJS() const
{
  int minSizeB = minSizeBefore().isAuto() 
    ? 0 : (int)minSizeBefore().toPixels();
  int maxSizeB = maxSizeBefore().isAuto() 
    ? 99999 : (int)maxSizeBefore().toPixels();
  int minSizeA = minSizeAfter().isAuto() 
    ? 0 : (int)minSizeAfter().toPixels();
  int maxSizeA = maxSizeAfter().isAuto()
    ? 99999 : (int)maxSizeAfter().toPixels();

  std::string Width;

  if (splitter_->orientation() == Horizontal) {
    Width = "Width";
  } else {
    Width = "Height";
  }

  /*
   * minSize = max(minSizeB, b.clientWidth + a.clientWidth - maxSizeA)
   * maxSize = min(maxSizeB, b.clientWidth + a.clientWidth - minSizeA)
   */

  WWidget *wb = splitter_->widgetBefore(this);
  WWidget *wa = splitter_->widgetAfter(this);

  return "{"
    "var w=" WT_CLASS ".getElement('" + wb->id() + "').client"
    + Width + "+"
    WT_CLASS ".getElement('" + wa->id() + "').client" + Width + ";"
    "var s=" + elVar() + ";"
    "s.minSize=Math.max(" + boost::lexical_cast<std::string>(minSizeB) +
    ",w+"
    + boost::lexical_cast<std::string>(splitter_->handleWidth()) + "-"
    + boost::lexical_cast<std::string>(maxSizeA) + ");"
    "s.maxSize=Math.min(" + boost::lexical_cast<std::string>(maxSizeB) +
    ",w-" + boost::lexical_cast<std::string>(minSizeA) + ");"
    "}";
}

  }
}
