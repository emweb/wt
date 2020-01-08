/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <ctype.h>

#include "Wt/WLineEdit.h"
#include "Wt/WApplication.h"
#include "Wt/WEnvironment.h"
#include "Wt/WStringUtil.h"
#include "Wt/WTheme.h"
#include "Wt/WLogger.h"

#include "DomElement.h"
#include "WebUtils.h"

#ifndef WT_DEBUG_JS
#include "js/WLineEdit.min.js"
#endif

namespace Wt {

LOGGER("WLineEdit");

const char *WLineEdit::INPUT_SIGNAL = "input";

WLineEdit::WLineEdit()
  : textSize_(10),
    maxLength_(-1),
    echoMode_(EchoMode::Normal),
    autoComplete_(true),
    maskChanged_(false),
    spaceChar_(' '),
    javaScriptDefined_(false)
{ 
  setInline(true);
  setFormObject(true);
}

WLineEdit::WLineEdit(const WT_USTRING& text)
  : textSize_(10),
    maxLength_(-1),
    echoMode_(EchoMode::Normal),
    autoComplete_(true),
    maskChanged_(false),
    spaceChar_(' '),
    javaScriptDefined_(false)
{
  setInline(true);
  setFormObject(true);
  setText(text);
}

void WLineEdit::setText(const WT_USTRING& text)
{
  WT_USTRING newDisplayText = inputText(text);
  WT_USTRING newText = removeSpaces(newDisplayText);
  if (maskChanged_ || content_ != newText || 
      displayContent_ != newDisplayText) {
    content_ = newText;
    displayContent_ = newDisplayText;

    if (isRendered() && !inputMask_.empty()) {
      doJavaScript(jsRef() + ".wtLObj"
	 ".setValue(" + WWebWidget::jsStringLiteral(newDisplayText) + ");");
    }

    flags_.set(BIT_CONTENT_CHANGED);
    repaint();

    validate();

    applyEmptyText();
  }
}

WT_USTRING WLineEdit::displayText() const
{
  if (echoMode_ == EchoMode::Normal) {
    return displayContent_;
  } else { // echoMode_ == Password
    std::u32string text = displayContent_;
#ifndef WT_TARGET_JAVA
    return WT_USTRING::fromUTF8(std::string(text.length(),'*'));
#else
    std::stringstream result;
    for (int i = 0; i < result.length(); i++) {
      result << '*';
    }
    return WT_USTRING::fromUTF8(result.str());
#endif
  }
}

void WLineEdit::setTextSize(int chars)
{
  if (textSize_ != chars) {
    textSize_ = chars;
    flags_.set(BIT_TEXT_SIZE_CHANGED);
    repaint(RepaintFlag::SizeAffected);
  }
}

void WLineEdit::setMaxLength(int chars)
{
  if (maxLength_ != chars) {
    maxLength_ = chars;
    flags_.set(BIT_MAX_LENGTH_CHANGED);
    repaint();
  }
}

void WLineEdit::setEchoMode(EchoMode echoMode)
{
  if (echoMode_ != echoMode) {
    echoMode_ = echoMode;
    flags_.set(BIT_ECHO_MODE_CHANGED);
    repaint();
  }
}

void WLineEdit::setAutoComplete(bool enabled)
{
  if (autoComplete_ != enabled) {
    autoComplete_ = enabled;
    flags_.set(BIT_AUTOCOMPLETE_CHANGED);
    repaint();
  }
}

void WLineEdit::updateDom(DomElement& element, bool all)
{
  if (all || flags_.test(BIT_CONTENT_CHANGED)) {
    WT_USTRING t = content_;
    if (!mask_.empty() && (inputMaskFlags_.test(
			   InputMaskFlag::KeepMaskWhileBlurred)))
      t = displayContent_;
    if (!all || !t.empty())
      element.setProperty(Wt::Property::Value, t.toUTF8());
    flags_.reset(BIT_CONTENT_CHANGED);
  }

  if (all || flags_.test(BIT_ECHO_MODE_CHANGED)) {
    element.setAttribute("type", echoMode_ == EchoMode::Normal 
			 ? "text" : "password");
    flags_.reset(BIT_ECHO_MODE_CHANGED);
  }

  if (all || flags_.test(BIT_AUTOCOMPLETE_CHANGED)) {
    if (!all || !autoComplete_) {
      element.setAttribute("autocomplete",
			   autoComplete_ == true ? "on" : "off");
    }
    flags_.reset(BIT_AUTOCOMPLETE_CHANGED);
  }

  if (all || flags_.test(BIT_TEXT_SIZE_CHANGED)) {
    element.setAttribute("size", std::to_string(textSize_));
    flags_.reset(BIT_TEXT_SIZE_CHANGED);
  }

  if (all || flags_.test(BIT_MAX_LENGTH_CHANGED)) {
    if (!all || maxLength_ > 0)
      element.setAttribute("maxLength", std::to_string(maxLength_));

    flags_.reset(BIT_MAX_LENGTH_CHANGED);
  }

  WFormWidget::updateDom(element, all);
}

void WLineEdit::getDomChanges(std::vector<DomElement *>& result,
			      WApplication *app)
{
  if (app->environment().agentIsIE() && flags_.test(BIT_ECHO_MODE_CHANGED)) {
    DomElement *e = DomElement::getForUpdate(this, domElementType());
    DomElement *d = createDomElement(app);

    app->theme()->apply(selfWidget(), *d, 0);

    e->replaceWith(d);
    result.push_back(e);
  } else
    WFormWidget::getDomChanges(result, app);
}

void WLineEdit::propagateRenderOk(bool deep)
{
  flags_.reset();

  WFormWidget::propagateRenderOk(deep);
}

DomElementType WLineEdit::domElementType() const
{
  return DomElementType::INPUT;
}

void WLineEdit::setFormData(const FormData& formData)
{
  // if the value was updated through the API, then ignore the update from
  // the browser, this happens when an action generated multiple events,
  // and we do not want to revert the changes made through the API
  if (flags_.test(BIT_CONTENT_CHANGED) || isReadOnly())
    return;

  if (!Utils::isEmpty(formData.values)) {
    const std::string& value = formData.values[0];
    displayContent_ = inputText(WT_USTRING::fromUTF8(value, true));
    content_ = removeSpaces(displayContent_);
  }
}

WT_USTRING WLineEdit::valueText() const
{
  return text();
}

void WLineEdit::setValueText(const WT_USTRING& value)
{
  setText(value);
}

int WLineEdit::boxPadding(Orientation orientation) const
{
  const WEnvironment& env = WApplication::instance()->environment();

  if (env.agentIsIE() || env.agentIsOpera())
    return 1;
  else if (env.agent() == UserAgent::Arora)
    return 0;
  else if (env.userAgent().find("Mac OS X") != std::string::npos)
    return 1;
  else if (env.userAgent().find("Windows") != std::string::npos
	   && !env.agentIsGecko())
    return 0;
  else
    return 1;
}

int WLineEdit::boxBorder(Orientation orientation) const
{
  const WEnvironment& env = WApplication::instance()->environment();

  if (env.userAgent().find("Mac OS X") != std::string::npos
      && env.agentIsGecko())
    return 3;
  else if (env.agent() == UserAgent::Arora)
    return 0;
  else
    return 2;
}

int WLineEdit::selectionStart() const
{
  WApplication *app = WApplication::instance();

  if (app->focus() == id()) {
    if (app->selectionStart() != -1
	&& app->selectionEnd() != app->selectionStart()) {
      return app->selectionStart();
    } else
      return -1;
  } else
    return -1;
}

WT_USTRING WLineEdit::selectedText() const
{
  if (selectionStart() != -1) {
    WApplication *app = WApplication::instance();

    std::string result = UTF8Substr(text().toUTF8(), app->selectionStart(),
				    app->selectionEnd() - app->selectionStart());
#ifdef WT_TARGET_JAVA
    return result;
#else
    return WString::fromUTF8(result);
#endif
  } else
    return WString::Empty;
}

bool WLineEdit::hasSelectedText() const
{
  return selectionStart() != -1;
}
  
void WLineEdit::setSelection(int start, int length)
{
  std::string s = std::to_string(start);
  std::string e = std::to_string(start + length);
  doJavaScript(WT_CLASS".setUnicodeSelectionRange(" + jsRef() + "," + s + "," + e + ")" );
}

int WLineEdit::cursorPosition() const
{
  WApplication *app = WApplication::instance();

  if (app->focus() == id())
    return app->selectionEnd();
  else
    return -1;
}

WT_USTRING WLineEdit::inputMask() const
{
  return inputMask_;
}

void WLineEdit::setInputMask(const WT_USTRING &mask,
			     WFlags<InputMaskFlag> flags)
{
  inputMaskFlags_ = flags;

  if (inputMask_ != mask) {
    inputMask_ = mask;
    mask_.clear();
    raw_.clear();
    case_.clear();
    spaceChar_ = ' ';
    WT_USTRING textBefore;
    if (!inputMask_.empty()) {
      textBefore = displayText();
      processInputMask();
      setText(textBefore);
    }

    if (isRendered() && javaScriptDefined_) {
      std::u32string space;
      space += spaceChar_;

      doJavaScript(jsRef() + ".wtLObj"
        ".setInputMask(" + WWebWidget::jsStringLiteral(mask_) + "," +
			   WWebWidget::jsStringLiteral(raw_) +  "," +
			   WWebWidget::jsStringLiteral(displayContent_) + "," +
			   WWebWidget::jsStringLiteral(case_) + "," +
			   WWebWidget::jsStringLiteral(space) + ", true);");
    } else if (!inputMask_.empty())
      repaint();
  }
}

void WLineEdit::render(WFlags<RenderFlag> flags)
{
  if (!mask_.empty() && !javaScriptDefined_)
    defineJavaScript();

  WFormWidget::render(flags);
}

// Remove spaces, only for input masks
WT_USTRING WLineEdit::removeSpaces(const WT_USTRING& text) const
{
  if (!raw_.empty() && !text.empty()) {
    std::u32string result = text;
    std::size_t i = 0;
    for (std::size_t j = 0; j < raw_.length(); ++i, ++j) {
      while (j < raw_.length() &&
	     result[j] == spaceChar_ &&
	     mask_[j] != '_') {
	++j;
      }
      if (j < raw_.length()) {
	if (i != j) {
	  result[i] = result[j];
	}
      } else {
	--i;
      }
    }
    result = result.substr(0, i);
    return WT_USTRING(result);
  } else {
    return text;
  }
}

// Unless the given text is empty, input the text as if it was
// entered character by character on the client side, applying
// the input mask, if present.
WT_USTRING WLineEdit::inputText(const WT_USTRING& text) const
{
  if (!raw_.empty() && !text.empty()) {
    std::u32string newText = text;
    std::u32string result = raw_;
    char32_t chr;
    bool hadIgnoredChar = false;
    std::size_t j = 0, i = 0;

    for (i = 0; i < newText.length(); ++i) {
      std::size_t previousJ = j;
      chr = newText[i];

      while (j < mask_.length() && !acceptChar(chr, j)) {
	++j; /* Try to move forward as long as this characer is not
	      * accepted in this position
	      */
      }
      if (j == mask_.length()) {
	j = previousJ;
	hadIgnoredChar = true;
      } else {
	if (raw_[j] != chr) {
	  if (case_[j] == '>') {
	    chr = toupper(chr);
	  } else if (case_[j] == '<') {
	    chr = tolower(chr);
	  }
	  result[j] = chr;
	}
	++j;
      }
    }
    if (hadIgnoredChar) {
      LOG_INFO("Input mask: not all characters in input '" + text + "' complied with "
	  "input mask " + inputMask_  + " and were ignored. Result is '" + result + "'.");
    }
    return WT_USTRING(result);
  }
  return text;
}

void WLineEdit::processInputMask() {
  if (inputMask_[inputMask_.length() - 2] == ';') {
    spaceChar_ = inputMask_[inputMask_.length() - 1];
    inputMask_ = inputMask_.substr(0, inputMask_.length() - 2);
  }

  mask_.reserve(inputMask_.length());
  raw_.reserve(inputMask_.length());
  case_.reserve(inputMask_.length());

  char mode = '!';
  for (std::size_t i = 0; i < inputMask_.length(); ++i) {
    char32_t currentChar = inputMask_[i];
    if (currentChar == '>' || currentChar == '<' || currentChar == '!') {
      mode = static_cast<char>(currentChar);
    } else if (std::u32string(U"AaNnXx90Dd#HhBb").find(currentChar) 
	       != std::u32string::npos) {
      mask_ += static_cast<char>(currentChar);
      raw_ += spaceChar_;
      case_ += mode;
    } else {
      if (currentChar == '\\')
	++i;
      mask_ += '_';
      raw_ += inputMask_[i];
      case_ += mode;
    }
  }
}

// Check whether the given character can be placed at the given
// position, according to the input mask.
bool WLineEdit::acceptChar(char32_t chr, std::size_t position) const {
  if (position >= mask_.length()) {
    return false;
  }
  if (raw_[position] == chr) {
    return true;
  }
  switch(mask_[position]) {
    case 'a':
    case 'A': // alphabetical: A-Za-z
      return (chr >= 'a' && chr <= 'z') || (chr >= 'A' && chr <= 'Z');
    case 'n':
    case 'N': // alphanumeric: A-Za-z0-9
      return (chr >= 'a' && chr <= 'z') || 
	(chr >= 'A' && chr <= 'Z') || 
	(chr >= '0' && chr <= '9');
    case 'x':
    case 'X': // Anything goes
      return true;
    case '0':
    case '9': // 0-9
      return chr >= '0' && chr <= '9';
    case 'd':
    case 'D': // 1-9
      return chr >= '1' && chr <= '9';
    case '#': // 0-9, + and -
      return (chr >= '0' && chr <= '9') || (chr == '-' || chr == '+');
    case 'h':
    case 'H': // hex
      return (chr >= 'A' && chr <= 'F') || 
	(chr >= 'a' && chr <= 'f') || 
	(chr >= '0' && chr <= '9');
    case 'b':
    case 'B': // binary
      return (chr == '0' || chr == '1');
  }
  return false;
}

EventSignal<>& WLineEdit::textInput()
{
  return *voidEventSignal(INPUT_SIGNAL, true);
}

void WLineEdit::defineJavaScript()
{
  if (javaScriptDefined_)
    return;
  javaScriptDefined_ = true;
  WApplication *app = WApplication::instance();

  LOAD_JAVASCRIPT(app, "js/WLineEdit.js", "WLineEdit", wtjs1);

  std::u32string space;
  space += spaceChar_;
  std::string jsObj = "new " WT_CLASS ".WLineEdit("
    + app->javaScriptClass() + "," + jsRef() + "," +
      WWebWidget::jsStringLiteral(mask_) + "," +
      WWebWidget::jsStringLiteral(raw_) +  "," +
      WWebWidget::jsStringLiteral(displayContent_) +  "," +
      WWebWidget::jsStringLiteral(case_) + "," +
      WWebWidget::jsStringLiteral(space) + "," +
    (inputMaskFlags_.test(InputMaskFlag::KeepMaskWhileBlurred) ? "0x1" : "0x0")
    + ");";

  setJavaScriptMember(" WLineEdit", jsObj);

#ifdef WT_CNOR
  EventSignalBase& b = mouseMoved();
  EventSignalBase& c = keyWentDown();
#endif

  connectJavaScript(keyWentDown(), "keyDown");
  connectJavaScript(keyPressed(), "keyPressed");
  connectJavaScript(focussed(), "focussed");
  connectJavaScript(blurred(), "blurred");
  connectJavaScript(clicked(), "clicked");
}

void WLineEdit::connectJavaScript(Wt::EventSignalBase& s,
				  const std::string& methodName)
{
  std::string jsFunction =
    "function(lobj, event) {"
    """var o = " + jsRef() + ";"
    """if (o && o.wtLObj) o.wtLObj." + methodName + "(lobj, event);"
    "}";

  s.connect(jsFunction);
}

ValidationState WLineEdit::validate()
{
  if (!inputMask_.empty() && !validateInputMask())
    return ValidationState::Invalid;
  else
    return WFormWidget::validate();
}

const std::string WLineEdit::SKIPPABLE_MASK_CHARS = "anx0d#hb";

// Simulates an NFA (non-deterministic finite state automaton)
// The states are positions in the input mask. We iterate over
// the input string once, going through all of the possible
// states in parallel, using the positions and nextPositions vector.
bool WLineEdit::validateInputMask() const {
  std::u32string toCheck = content_;
  if (toCheck.empty()) {
    toCheck = raw_;
  }

  // Switch between two vectors without copy assignment.
  std::vector<std::size_t> p1;
  std::vector<std::size_t> p2;

  std::vector<std::size_t> *positions = &p1;
  std::vector<std::size_t> *nextPositions = &p2;

  positions->push_back(0);
  for (std::size_t i = 0; i < toCheck.length(); ++i) {
    for (std::size_t j = 0; j < positions->size(); ++j) {
      std::size_t currentPosition = (*positions)[j];
      if (currentPosition < mask_.length()) {
	// Check whether we can skip the current position, if so, add
	// it to positions vector, to be considered later (with the current
	// input character still).
	if (SKIPPABLE_MASK_CHARS.find(mask_[currentPosition]) 
	    != std::string::npos &&
	    (j + 1 == positions->size() || 
	     (*positions)[j + 1] != currentPosition + 1)) {
	  positions->push_back(currentPosition + 1);
	}

	// Check whether we can accept the current character in the current
	// position, if so, the next position is added to the nextPositions
	// vector.
	if (acceptChar(toCheck[i], currentPosition) &&
	    (nextPositions->empty() || 
	     nextPositions->back() != currentPosition + 1)) {
	  nextPositions->push_back(currentPosition + 1);
	}
      }
    }
    std::swap(positions, nextPositions);
    nextPositions->clear();
    if (positions->size() == 0) {
      return false;
    }
  }
  while (positions->size() > 0) {
    for (std::size_t j = 0; j < positions->size(); ++j) {
      std::size_t currentPosition = (*positions)[j];
      // One path is in the end state, accept.
      if (currentPosition == mask_.length()) {
	return true;
      }
      // Check whether we can skip the rest of the mask.
      if (SKIPPABLE_MASK_CHARS.find(mask_[currentPosition]) 
	  != std::string::npos &&
	  (nextPositions->empty() || 
	   nextPositions->back() != currentPosition + 1)) {
	nextPositions->push_back(currentPosition + 1);
      }
    }
    std::swap(positions, nextPositions);
    nextPositions->clear();
  }
  return false;
}

}
