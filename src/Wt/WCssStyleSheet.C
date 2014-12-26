/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WCssStyleSheet"
#include "Wt/WApplication"
#include "Wt/WEnvironment"

#include "DomElement.h"
#include "EscapeOStream.h"
#include "WebUtils.h"

namespace Wt {

class WCssTemplateWidget : public WWebWidget
{
public:
  WCssTemplateWidget(WCssTemplateRule *rule)
    : rule_(rule)
  { }

  virtual void setPositionScheme(PositionScheme scheme) {
    WWebWidget::setPositionScheme(scheme);
    rule_->modified();
  }

  virtual void setOffsets(const WLength& offset, WFlags<Side> sides = All) {
    WWebWidget::setOffsets(offset, sides);
    rule_->modified();
  }

  virtual void resize(const WLength& width, const WLength& height) {
    WWebWidget::resize(width, height);
    rule_->modified();
  }

  virtual void setMinimumSize(const WLength& width, const WLength& height) {
    WWebWidget::setMinimumSize(width, height);
    rule_->modified();
  }

  virtual void setMaximumSize(const WLength& width, const WLength& height) {
    WWebWidget::setMaximumSize(width, height);
    rule_->modified();
  }

  virtual void setLineHeight(const WLength& height) {
    WWebWidget::setLineHeight(height);
    rule_->modified();
  }

  virtual void setFloatSide(Side s) {
    WWebWidget::setFloatSide(s);
    rule_->modified();
  }

  virtual void setClearSides(WFlags<Side> sides) {
    WWebWidget::setClearSides(sides);
    rule_->modified();
  }

  virtual void setMargin(const WLength& margin, WFlags<Side> sides = All) {
    WWebWidget::setMargin(margin, sides);
    rule_->modified();
  }

  virtual void setHidden(bool hidden, const WAnimation& animation = WAnimation()) {
    WWebWidget::setHidden(hidden, animation);
    rule_->modified();
  }

  virtual void setPopup(bool popup) {
    WWebWidget::setPopup(popup);
    rule_->modified();
  }

  virtual void setInline(bool isinline) {
    WWebWidget::setInline(isinline);
    rule_->modified();
  }

  virtual WCssDecorationStyle& decorationStyle() {
    // Assumption here! We should really catch modifications to the
    // stylesheet...
    rule_->modified();

    return WWebWidget::decorationStyle();
  }

  virtual void setVerticalAlignment(AlignmentFlag alignment,
				    const WLength& length) {
    WWebWidget::setVerticalAlignment(alignment, length);
    rule_->modified();
  }

  virtual DomElementType domElementType() const { return DomElement_SPAN; }

private:
  WCssTemplateRule *rule_;
};

WCssRule::WCssRule(const std::string& selector, WObject* parent)
  : WObject(parent),
    selector_(selector),
    sheet_(0)
{ }

WCssRule::~WCssRule()
{ 
  if (sheet_)
    sheet_->removeRule(this);
}

void WCssRule::modified()
{
  if (sheet_)
    sheet_->ruleModified(this);
}

bool WCssRule::updateDomElement(DomElement& cssRuleElement, bool all)
{
  return false;
}

WCssTemplateRule::WCssTemplateRule(const std::string& selector, 
				   WObject* parent)
  : WCssRule(selector, parent)
{
  widget_ = new WCssTemplateWidget(this);
}

WCssTemplateRule::~WCssTemplateRule()
{
  delete widget_;
}

WWidget *WCssTemplateRule::templateWidget()
{
  return widget_;
}

std::string WCssTemplateRule::declarations()
{
  DomElement e(DomElement::ModeUpdate, widget_->domElementType());
  updateDomElement(e, true);
  return e.cssStyle();
}

bool WCssTemplateRule::updateDomElement(DomElement& element, bool all)
{
  widget_->updateDom(element, all);
  return true;
}

WCssTextRule::WCssTextRule(const std::string& selector,
			   const WT_USTRING& declarations,
			   WObject* parent)
  : WCssRule(selector, parent),
    declarations_(declarations)
{ }

std::string WCssTextRule::declarations()
{
  return declarations_.toUTF8(); 
}

WCssStyleSheet::WCssStyleSheet()
{ }

WCssStyleSheet::WCssStyleSheet(const WLink& link, const std::string& media)
  : link_(link),
    media_(media)
{ }

WCssStyleSheet::~WCssStyleSheet()
{
  while (!rules_.empty())
    delete rules_.back();
}

WCssRule *WCssStyleSheet::addRule(WCssRule *rule, const std::string& ruleName)
{
  rules_.push_back(rule);
  rulesAdded_.push_back(rule);
  rule->sheet_ = this;

  if (!ruleName.empty())
    defined_.insert(ruleName);

  return rule;
}

WCssTemplateRule *WCssStyleSheet::addRule(const std::string& selector,
					  const WCssDecorationStyle& style,
					  const std::string& ruleName)
{
  WCssTemplateRule *result = new WCssTemplateRule(selector);
  result->templateWidget()->setDecorationStyle(style);

  addRule(result, ruleName);
  return result;
}

WCssTextRule *WCssStyleSheet::addRule(const std::string& selector,
				      const WT_USTRING& declarations,
				      const std::string& ruleName)
{
  WCssTextRule *result = new WCssTextRule(selector, declarations);
  addRule(result, ruleName);
  return result;
}

#ifndef WT_TARGET_JAVA
WCssTextRule *WCssStyleSheet::addRule(const std::string& selector,
				      const char *declarations,
				      const std::string& ruleName)
{
  return addRule(selector, WT_USTRING::fromUTF8(declarations), ruleName);
}

WCssTextRule *WCssStyleSheet::addRule(const std::string& selector,
				      const std::string& declarations,
				      const std::string& ruleName)
{
  return addRule(selector, WT_USTRING::fromUTF8(declarations), ruleName);
}
#endif

bool WCssStyleSheet::isDefined(const std::string& ruleName) const
{
  std::set<std::string>::const_iterator i = defined_.find(ruleName);
  return i != defined_.end();
}

void WCssStyleSheet::removeRule(WCssRule *rule)
{
  if (Utils::erase(rules_, rule)) {
    if (!Utils::erase(rulesAdded_, rule))
      rulesRemoved_.push_back(rule->selector());

    rulesModified_.erase(rule);
  }
}

void WCssStyleSheet::ruleModified(WCssRule *rule)
{
  rulesModified_.insert(rule);
}

void WCssStyleSheet::cssText(WStringStream& out, bool all)
{
  if (link_.isNull()) {
    RuleList& toProcess = all ? rules_ : rulesAdded_;

    for (unsigned i = 0; i < toProcess.size(); ++i) {
      WCssRule *rule = toProcess[i];
      out << rule->selector() << " { " << rule->declarations() << " }\n";
    }

    rulesAdded_.clear();

    if (all)
      rulesModified_.clear();
  } else {
    WApplication *app = WApplication::instance();
    out << "@import url(\"" << link_.resolveUrl(app) << "\")";

    if (!media_.empty() && media_ != "all")
      out << " " << media_;
    out << ";\n";
  }
}

void WCssStyleSheet::javaScriptUpdate(WApplication *app,
				      WStringStream& js, bool all)
{
  if (!all) {
    for (unsigned i = 0; i < rulesRemoved_.size(); ++i) {
      js << WT_CLASS ".removeCssRule(";
      DomElement::jsStringLiteral(js, rulesRemoved_[i], '\'');
      js << ");";
    }
    rulesRemoved_.clear();

    for (RuleSet::const_iterator i = rulesModified_.begin();
	 i != rulesModified_.end(); ++i) {
      js << "{ var d= " WT_CLASS ".getCssRule(";
      DomElement::jsStringLiteral(js, (*i)->selector(), '\'');
      js << ");if(d){";

      DomElement *d = DomElement::updateGiven("d", DomElement_SPAN);
      if ((*i)->updateDomElement(*d, false)) {
	EscapeOStream sout(js);
	d->asJavaScript(sout, DomElement::Update);
      }

      delete d;

      js << "}}";
    }
    rulesModified_.clear();
  }

  if (!app->environment().agentIsIElt(9)
      && app->environment().agent() != WEnvironment::Konqueror) {
    RuleList& toProcess = all ? rules_ : rulesAdded_;

    for (unsigned i = 0; i < toProcess.size(); ++i) {
      WCssRule *rule = toProcess[i];
      js << WT_CLASS ".addCss('"
	 << rule->selector() << "',";
      DomElement::jsStringLiteral(js, rule->declarations(), '\'');
      js << ");\n";
    }

    rulesAdded_.clear();
    if (all)
      rulesModified_.clear();
  } else {
    WStringStream css;
    cssText(css, all);
    if (!css.empty()) {
      js << WT_CLASS ".addCssText(";
      DomElement::jsStringLiteral(js, css.str(), '\'');
      js << ");\n";
    }
  }
}

void WCssStyleSheet::clear()
{
  while (!rules_.empty())
    delete rules_.back();
}

bool WCssStyleSheet::isDirty()
{
  return !rulesAdded_.empty() || !rulesModified_.empty() ||
      !rulesRemoved_.empty();
}

} // namespace Wt
