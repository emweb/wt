/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <Wt/WApplication>
#include <Wt/WDoubleValidator>
#include <Wt/WIntValidator>
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
     step_(1),
     changed_(false)
{
  setStyleClass("Wt-spinbox");

  changed().connect(this, &WSpinBox::onChange);

#ifdef WT_CNOR // ??
  EventSignalBase& b = mouseMoved();
  EventSignalBase& c = keyWentDown();
#endif

  connectJavaScript(mouseMoved(), "mouseMove");
  connectJavaScript(mouseWentUp(), "mouseUp");
  connectJavaScript(mouseWentDown(), "mouseDown");
  connectJavaScript(mouseWentOut(), "mouseOut");
  connectJavaScript(keyWentDown(), "keyDown");

  updateValidator();

  setValue(0);
}

void WSpinBox::setValue(double value)
{
  if (static_cast<int>(value) == value)
    setText(WT_USTRING::fromUTF8(boost::lexical_cast<std::string>
				 (static_cast<int>(value))));
  else
    setText(WT_USTRING::fromUTF8(boost::lexical_cast<std::string>(value)));

  valueChanged_.emit(value);

  changed_ = true;
  repaint(RepaintInnerHtml);
}

double WSpinBox::value() const
{
  try {
    return boost::lexical_cast<double>(text().toUTF8());
  } catch (const boost::bad_lexical_cast& e) {
    return 0;
  }
}

void WSpinBox::setMinimum(double minimum)
{
  min_ = minimum;

  changed_ = true;
  repaint(RepaintInnerHtml);

  updateValidator();
}

void WSpinBox::setMaximum(double maximum)
{
  max_ = maximum;

  changed_ = true;
  repaint(RepaintInnerHtml);

  updateValidator();
}

void WSpinBox::setRange(double minimum, double maximum)
{
  min_ = minimum;
  max_ = maximum;

  changed_ = true;
  repaint(RepaintInnerHtml);

  updateValidator();
}

void WSpinBox::setSingleStep(double step)
{
  step_ = step;

  changed_ = true;
  repaint(RepaintInnerHtml);

  updateValidator();
}

void WSpinBox::updateValidator()
{
  WValidator *v = validator();

  if (!v)
    setValidator(new WDoubleValidator(min_, max_));
  else {
    WDoubleValidator *dv = dynamic_cast<WDoubleValidator *>(v);
    dv->setRange(min_, max_);
  }
}

void WSpinBox::updateDom(DomElement& element, bool all)
{
  if (changed_) {
    element.callJavaScript("jQuery.data(" + jsRef() + ", 'obj').update("
			   + boost::lexical_cast<std::string>(min_)
			   + ','
			   + boost::lexical_cast<std::string>(max_)
			   + ','
			   + boost::lexical_cast<std::string>(step_)
			   + ");");

    changed_ = false;
  }

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
  changed_ = false;
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
  valueChanged_.emit(value());
}

}

