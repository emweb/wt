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

#include <boost/algorithm/string.hpp>

#ifndef WT_DEBUG_JS
#include "js/WSpinBox.min.js"
#endif

namespace Wt {

class SpinBoxValidator : public WValidator
{
public:
  SpinBoxValidator(WAbstractSpinBox *spinBox)
    : spinBox_(spinBox)
  { }

  virtual Result validate(const WT_USTRING& input) const {
    bool valid = spinBox_->parseValue(input);
    if (valid)
      return spinBox_->validateRange();
    else
      return Result(Invalid);
  }
 
  virtual std::string javaScriptValidate() const {
    return 
      "new function() { "
      """this.validate = function(t) {"
      ""  "return jQuery.data(" + spinBox_->jsRef() + ", 'obj').validate(t);"
      """};"
      "}";
  }

private:
  WAbstractSpinBox *spinBox_;
};

WAbstractSpinBox::WAbstractSpinBox(WContainerWidget *parent)
  : WLineEdit(parent),
    changed_(false),
    valueChangedConnection_(false),
    preferNative_(false),
    setup_(false)
{ }

void WAbstractSpinBox::setNativeControl(bool nativeControl)
{
  preferNative_ = nativeControl;
}

bool WAbstractSpinBox::nativeControl() const
{
  if (preferNative_) {
    if (!WLineEdit::inputMask().empty()) {
      return false;
    }

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
  if (prefix_ != prefix) {
    prefix_ = prefix;
    setText(textFromValue());
    changed_ = true;
    repaint();
  }
}

void WAbstractSpinBox::setSuffix(const WString& suffix)
{
  if (suffix_ != suffix) {
    suffix_ = suffix;
    setText(textFromValue());
    changed_ = true;
    repaint();
  }
}

void WAbstractSpinBox::render(WFlags<RenderFlag> flags)
{
  /*
   * In theory we are a bit late here to decide what we want to become:
   * somebody could already have asked the domElementType()
   */
  if (!setup_ && flags & RenderFull) {
    setup();
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

  LOAD_JAVASCRIPT(app, "js/WSpinBox.js", "WSpinBox", wtjs1);

  std::string jsObj = "new " WT_CLASS ".WSpinBox("
    + app->javaScriptClass() + "," + jsRef() + ","
    + boost::lexical_cast<std::string>(decimals()) + ","
    + prefix().jsStringLiteral() + ","
    + suffix().jsStringLiteral() + ","
    + jsMinMaxStep() + ","
    + jsStringLiteral(WLocale::currentLocale().decimalPoint()) + ","
    + jsStringLiteral(WLocale::currentLocale().groupSeparator()) + ");";

  setJavaScriptMember(" WSpinBox", jsObj);
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
	doJavaScript("jQuery.data(" + jsRef() + ", 'obj')"
		     ".configure("
		     + boost::lexical_cast<std::string>(decimals()) + ","
		     + prefix().jsStringLiteral() + ","
		     + suffix().jsStringLiteral() + ","
		     + jsMinMaxStep()+ ");");
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

void WAbstractSpinBox::setup()
{
  setup_ = true;
  bool useNative = nativeControl();

  if (!useNative) {
    defineJavaScript();

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

    if (!prefix_.empty() || !suffix_.empty())
      setValidator(new SpinBoxValidator(this));
  }
}

WValidator::State WAbstractSpinBox::validate()
{
  return WLineEdit::validate();
}

void WAbstractSpinBox::refresh()
{
  doJavaScript
    ("jQuery.data(" + jsRef() + ", 'obj')"
     ".setLocale(" 
     + jsStringLiteral(WLocale::currentLocale().decimalPoint()) + ","
     + jsStringLiteral(WLocale::currentLocale().groupSeparator()) + ");");

  WLineEdit::refresh();
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
    valid = textUtf8.length() > 0;

  if (valid)
    valid = parseNumberValue(textUtf8);

  return valid;
}


}
