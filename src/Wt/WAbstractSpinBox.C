/*
 * Copyright (C) 2011 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WAbstractSpinBox"
#include "Wt/WApplication"
#include "Wt/WEnvironment"
#include "Wt/WValidator"

#include "DomElement.h"
#include "JavaScriptLoader.h"

#ifndef WT_DEBUG_JS
#include "js/WSpinBox.min.js"
#endif

#include <boost/algorithm/string.hpp>

namespace Wt {

class SpinBoxValidator : public WValidator
{
public:
  SpinBoxValidator(WAbstractSpinBox *spinBox)
    : spinBox_(spinBox)
  { }

  virtual State validate(WT_USTRING& input) const {
    return spinBox_->parseValue(input) ? Valid : Invalid;
  }

  virtual std::string javaScriptValidate() const {
    return "jQuery.data(" + spinBox_->jsRef() + ", 'obj');";
  }

private:
  WAbstractSpinBox *spinBox_;
};

WAbstractSpinBox::WAbstractSpinBox(WContainerWidget *parent)
  : WLineEdit(parent),
    changed_(false),
    valueChangedConnection_(false),
    preferNative_(false)
{ }

void WAbstractSpinBox::setNativeControl(bool nativeControl)
{
  preferNative_ = nativeControl;
}

bool WAbstractSpinBox::nativeControl() const
{
  if (preferNative_) {
    const WEnvironment& env = WApplication::instance()->environment();
    if ((env.agentIsChrome() && env.agent() >= WEnvironment::Chrome5)
	|| (env.agentIsSafari() && env.agent() >= WEnvironment::Safari4)
	|| (env.agentIsOpera() && env.agent() >= WEnvironment::Opera10))
      return true;
  }

  return false;
}

void WAbstractSpinBox::setPrefix(const WString& prefix)
{
  prefix_ = prefix;
}

void WAbstractSpinBox::setSuffix(const WString& suffix)
{
  suffix_ = suffix;
}

void WAbstractSpinBox::render(WFlags<RenderFlag> flags)
{
  /*
   * In theory we are a bit late here to decide what we want to become:
   * somebody could already have asked the domElementType()
   */
  if (flags & RenderFull) {
    bool useNative = nativeControl();

    setup(useNative);
  }

  WLineEdit::render(flags);
}

void WAbstractSpinBox::connectJavaScript(Wt::EventSignalBase& s,
					 const std::string& methodName)
{
  std::string jsFunction = 
    "function(obj, event) {"
    """var o = jQuery.data(" + jsRef() + ", 'obj');"
    """if (o) o." + methodName + "(obj, event);"
    "}";

  s.connect(jsFunction);
}

void WAbstractSpinBox::defineJavaScript()
{
  WApplication *app = WApplication::instance();

  const char *THIS_JS = "js/WSpinBox.js";

  if (!app->javaScriptLoaded(THIS_JS)) {
    LOAD_JAVASCRIPT(app, THIS_JS, "WSpinBox", wtjs1);
    app->setJavaScriptLoaded(THIS_JS);
  }

  std::string jsObj = "new " WT_CLASS ".WSpinBox("
    + app->javaScriptClass() + "," + jsRef() + ","
    + boost::lexical_cast<std::string>(decimals()) + ","
    + prefix().jsStringLiteral() + ","
    + suffix().jsStringLiteral() + ","
    + jsMinMaxStep() + ");";

  setJavaScriptMember("_a", "0;" + jsObj);
}

void WAbstractSpinBox::setText(const WT_USTRING& text)
{
  parseValue(text);

  WLineEdit::setText(textFromValue());
}

void WAbstractSpinBox::setFormData(const FormData& formData)
{
  WLineEdit::setFormData(formData);

  parseValue(text());
}

void WAbstractSpinBox::updateDom(DomElement& element, bool all)
{
  if (all || changed_) {
    if (!all) {
      if (!nativeControl())
	WApplication::instance()->doJavaScript
	  ("jQuery.data(" + jsRef() + ", 'obj')"
	   ".update(" + jsMinMaxStep() + ");");
      else
	setValidator(createValidator());
    }
  }

  changed_ = false;

  WLineEdit::updateDom(element, all);

  if (all && nativeControl())
    element.setAttribute("type", "number");
}

void WAbstractSpinBox::propagateRenderOk(bool deep)
{
  changed_ = false;

  WLineEdit::propagateRenderOk(deep);
}

void WAbstractSpinBox::setup(bool useNative)
{
  if (useNative) {
    setValidator(createValidator());
  } else {
    defineJavaScript();

    addStyleClass("Wt-spinbox");

#ifdef WT_CNOR
    EventSignalBase& b = mouseMoved();
    EventSignalBase& c = keyWentDown();
#endif

    connectJavaScript(mouseMoved(), "mouseMove");
    connectJavaScript(mouseWentUp(), "mouseUp");
    connectJavaScript(mouseWentDown(), "mouseDown");
    connectJavaScript(mouseWentOut(), "mouseOut");
    connectJavaScript(keyWentDown(), "keyDown");
    connectJavaScript(keyWentUp(), "keyUp");

    setValidator(new SpinBoxValidator(this));
  }
}

int WAbstractSpinBox::boxPadding(Orientation orientation) const
{
  if (!nativeControl() && orientation == Horizontal)
    return WLineEdit::boxPadding(orientation) + 8; // Half since for one side
  else
    return WLineEdit::boxPadding(orientation);
}

bool WAbstractSpinBox::parseValue(const WT_USTRING& text)
{
  std::string textUtf8 = text.toUTF8();

  bool valid = true;

  if (!nativeControl()) {
    valid = false;

    std::string prefixUtf8 = prefix_.toUTF8();
    std::string suffixUtf8 = suffix_.toUTF8();

    if (boost::starts_with(textUtf8, prefixUtf8)) {
      textUtf8 = textUtf8.substr(prefixUtf8.length());
      if (boost::ends_with(textUtf8, suffixUtf8)) {
	textUtf8 = textUtf8.substr(0, textUtf8.length() - suffixUtf8.length());
	valid = true;
      }
    }
  }

  if (valid)
    valid = parseNumberValue(textUtf8);

  return valid;
}


}
