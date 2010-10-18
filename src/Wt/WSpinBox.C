/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <Wt/WApplication>
#include <Wt/WSpinBox>

#include "DomElement.h"

#include "JavaScriptLoader.h"

#ifndef WT_DEBUG_JS
#include "js/WSpinBox.min.js"
#endif

using namespace Wt;

namespace Wt {

WSpinBox::WSpinBox(WContainerWidget *parent)
   : WLineEdit(parent),
     min_(0),
     max_(99),
     value_(0),
     step_(1),
     changed_(false)
{
  setStyleClass("Wt-spinbox");

  changed().connect(this, &WSpinBox::onChange);

  connectJavaScript(mouseMoved(), "mouseMove");
  connectJavaScript(mouseWentUp(), "mouseUp");
  connectJavaScript(mouseWentDown(), "mouseDown");
}

void WSpinBox::setValue(double value)
{
  value_ = value;

  valueChanged_.emit(value_);

  changed_ = true;
  repaint(RepaintInnerHtml);
}

void WSpinBox::setMinimum(double minimum)
{
  min_ = minimum;

  changed_ = true;
  repaint(RepaintInnerHtml);
}

void WSpinBox::setMaximum(double maximum)
{
  max_ = maximum;

  changed_ = true;
  repaint(RepaintInnerHtml);
}

void WSpinBox::setRange(double minimum, double maximum)
{
  min_ = minimum;
  max_ = maximum;

  changed_ = true;
  repaint(RepaintInnerHtml);
}

void WSpinBox::updateDom(DomElement& element, bool all)
{
  WLineEdit::updateDom(element, all);
}

void WSpinBox::propagateRenderOk(bool deep)
{
  changed_ = false;

  WLineEdit::propagateRenderOk(deep);
}

void WSpinBox::defineJavaScript()
{
  WApplication *app = WApplication::instance();

  const char *THIS_JS = "js/WSpinBox.js";

  if (!app->javaScriptLoaded(THIS_JS)) {
    LOAD_JAVASCRIPT(app, THIS_JS, "WSpinBox", wtjs1);
    app->setJavaScriptLoaded(THIS_JS);
  }

  app->doJavaScript("new " WT_CLASS ".WSpinBox("
		    + app->javaScriptClass() + "," + jsRef() + ","
		    + boost::lexical_cast<std::string>(min_) + ","
		    + boost::lexical_cast<std::string>(max_) + ","
		    + boost::lexical_cast<std::string>(step_) + ");");
}

void WSpinBox::connectJavaScript(Wt::EventSignalBase& s,
				 const std::string& methodName)
{
  std::string jsFunction = 
    "function(obj, event) {"
    """var o = jQuery.data(" + jsRef() + ", 'obj');"
    """if (o) o." + methodName + "(obj, event);"
    "}";
  s.connect(jsFunction);
}

void WSpinBox::render(WFlags<RenderFlag> flags)
{
  if (flags & RenderFull)
    defineJavaScript();

  WLineEdit::render(flags);
}

void WSpinBox::onChange()
{
}

}

