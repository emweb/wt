/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WAbstractSpinBox.h"
#include "Wt/WApplication.h"
#include "Wt/WEnvironment.h"
#include "Wt/WValidator.h"

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

  virtual Result validate(const WT_USTRING& input) const override {
    bool valid = spinBox_->parseValue(input);
    if (valid)
      return spinBox_->validateRange();
    else
      return Result(ValidationState::Invalid);
  }
 
  virtual std::string javaScriptValidate() const override {
    return 
      "new function() { "
      """this.validate = function(t) {"
      ""  "return " + spinBox_->jsRef() + ".wtObj.validate(t);"
      """};"
      "}";
  }

private:
  WAbstractSpinBox *spinBox_;
};

WAbstractSpinBox::WAbstractSpinBox()
  : changed_(false),
    valueChangedConnection_(false),
    preferNative_(false),
    setup_(false),
    jsValueChanged_(this, "spinboxValueChanged", true)
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
    if ((env.agentIsChrome() && 
	 static_cast<unsigned int>(env.agent()) >= 
	 static_cast<unsigned int>(UserAgent::Chrome5))
	|| (env.agentIsSafari() && 
	    static_cast<unsigned int>(env.agent()) >= 
	    static_cast<unsigned int>(UserAgent::Safari4))
	|| (env.agentIsOpera() && 
	    static_cast<unsigned int>(env.agent()) >=
	    static_cast<unsigned int>(UserAgent::Opera10)))
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
  if (!setup_ && flags.test(RenderFlag::Full)) {
    setup();
  }

  if (jsValueChanged().needsUpdate(true)) {
    WStringStream function;
    function << jsRef() << ".wtObj.jsValueChanged=";
    if (jsValueChanged().isConnected()) {
      function << "function(oldv, v){"
#ifndef WT_TARGET_JAVA
               << "var o=null;var e=null;" << jsValueChanged().createCall({"oldv", "v"}) << "};";
#else // WT_TARGET_JAVA
               << "var o=null;var e=null;" << jsValueChanged().createCall("oldv", "v") << "};";
#endif // WT_TARGET_JAVA
    } else {
      function << "function() {};";
    }
    doJavaScript(function.str());
  }

  WLineEdit::render(flags);
}

void WAbstractSpinBox::connectJavaScript(Wt::EventSignalBase& s,
					 const std::string& methodName)
{
  std::string jsFunction = 
    "function(obj, event) {"
    """var o = " + jsRef() + ";"
    """if (o && o.wtObj) o.wtObj." + methodName + "(obj, event);"
    "}";

  s.connect(jsFunction);
}

void WAbstractSpinBox::defineJavaScript()
{
  WApplication *app = WApplication::instance();

  LOAD_JAVASCRIPT(app, "js/WSpinBox.js", "WSpinBox", wtjs1);

  WStringStream ss;

  ss << "new " WT_CLASS ".WSpinBox("
    << app->javaScriptClass() << "," << jsRef() << "," << decimals() << ","
    << prefix().jsStringLiteral() << ","
    << suffix().jsStringLiteral() << ","
    << jsMinMaxStep() << ","
    << jsStringLiteral(WLocale::currentLocale().decimalPoint()) << ","
    << jsStringLiteral(WLocale::currentLocale().groupSeparator())
    << ");";

  setJavaScriptMember(" WSpinBox", ss.str());
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
	doJavaScript(jsRef() + ".wtObj"
		     ".configure("
		     + std::to_string(decimals()) + ","
		     + prefix().jsStringLiteral() + ","
		     + suffix().jsStringLiteral() + ","
		     + jsMinMaxStep() + ");");
      else
	setValidator(std::shared_ptr<WValidator>(createValidator().release()));
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
      setValidator(std::shared_ptr<SpinBoxValidator>(new SpinBoxValidator(this)));
  }
}

ValidationState WAbstractSpinBox::validate()
{
  return WLineEdit::validate();
}

void WAbstractSpinBox::refresh()
{
  doJavaScript
    (jsRef() + ".wtObj"
     ".setLocale(" 
     + jsStringLiteral(WLocale::currentLocale().decimalPoint()) + ","
     + jsStringLiteral(WLocale::currentLocale().groupSeparator()) + ");");

  WLineEdit::refresh();
}

int WAbstractSpinBox::boxPadding(Orientation orientation) const
{
  if (!nativeControl() && orientation == Orientation::Horizontal)
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
