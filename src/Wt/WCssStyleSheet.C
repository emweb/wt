/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WCssStyleSheet.h"
#include "Wt/WApplication.h"
#include "Wt/WEnvironment.h"

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

  virtual void setPositionScheme(PositionScheme scheme) override {
    WWebWidget::setPositionScheme(scheme);
    rule_->modified();
  }

  virtual void setOffsets(const WLength& offset,
			  WFlags<Side> sides = AllSides) override {
    WWebWidget::setOffsets(offset, sides);
    rule_->modified();
  }

  virtual void resize(const WLength& width, const WLength& height) override {
    WWebWidget::resize(width, height);
    rule_->modified();
  }

  virtual void setMinimumSize(const WLength& width, const WLength& height) 
    override
  {
    WWebWidget::setMinimumSize(width, height);
    rule_->modified();
  }

  virtual void setMaximumSize(const WLength& width, const WLength& height) 
    override
  {
    WWebWidget::setMaximumSize(width, height);
    rule_->modified();
  }

  virtual void setLineHeight(const WLength& height) override
  {
    WWebWidget::setLineHeight(height);
    rule_->modified();
  }

  virtual void setFloatSide(Side s) override {
    WWebWidget::setFloatSide(s);
    rule_->modified();
  }

  virtual void setClearSides(WFlags<Side> sides) override {
    WWebWidget::setClearSides(sides);
    rule_->modified();
  }

  virtual void setMargin(const WLength& margin,
			 WFlags<Side> sides = AllSides) override {
    WWebWidget::setMargin(margin, sides);
    rule_->modified();
  }

  virtual void setHidden(bool hidden, 
			 const WAnimation& animation = WAnimation()) override {
    WWebWidget::setHidden(hidden, animation);
    rule_->modified();
  }

  virtual void setPopup(bool popup) override {
    WWebWidget::setPopup(popup);
    rule_->modified();
  }

  virtual void setInline(bool isinline) override {
    WWebWidget::setInline(isinline);
    rule_->modified();
  }

  virtual WCssDecorationStyle& decorationStyle() override {
    // Assumption here! We should really catch modifications to the
    // stylesheet...
    rule_->modified();

    return WWebWidget::decorationStyle();
  }

  virtual void setVerticalAlignment(AlignmentFlag alignment,
				    const WLength& length) override {
    WWebWidget::setVerticalAlignment(alignment, length);
    rule_->modified();
  }

  virtual DomElementType domElementType() const override { 
    return DomElementType::SPAN; 
  }

private:
  WCssTemplateRule *rule_;
};

WCssRule::WCssRule(const std::string& selector)
  : selector_(selector),
    sheet_(nullptr)
{ }

WCssRule::~WCssRule()
{ }

void WCssRule::modified()
{
  if (sheet_)
    sheet_->ruleModified(this);
}

bool WCssRule::updateDomElement(DomElement& cssRuleElement, bool all)
{
  return false;
}

WCssTemplateRule::WCssTemplateRule(const std::string& selector)
  : WCssRule(selector)
{
  widget_.reset(new WCssTemplateWidget(this));
}

WCssTemplateRule::~WCssTemplateRule()
{ }

WWidget *WCssTemplateRule::templateWidget()
{
  return widget_.get();
}

std::string WCssTemplateRule::declarations()
{
  DomElement e(DomElement::Mode::Update, widget_->domElementType());
  updateDomElement(e, true);
  return e.cssStyle();
}

bool WCssTemplateRule::updateDomElement(DomElement& element, bool all)
{
  widget_->updateDom(element, all);
  return true;
}

WCssTextRule::WCssTextRule(const std::string& selector,
			   const WT_USTRING& declarations)
  : WCssRule(selector),
    declarations_(declarations)
{ }

std::string WCssTextRule::declarations()
{
  return declarations_.toUTF8(); 
}

WCssStyleSheet::WCssStyleSheet()
{ }

WCssStyleSheet::~WCssStyleSheet()
{ }

WCssRule *WCssStyleSheet::addRule(std::unique_ptr<WCssRule> rule,
				  const std::string& ruleName)
{
  rule->sheet_ = this;

  rulesAdded_.push_back(rule.get());
  rules_.push_back(std::move(rule));

  if (!ruleName.empty())
    defined_.insert(ruleName);

  return rules_.back().get();
}

WCssTemplateRule *WCssStyleSheet::addRule(const std::string& selector,
					  const WCssDecorationStyle& style,
					  const std::string& ruleName)
{
  std::unique_ptr<WCssTemplateRule> r(new WCssTemplateRule(selector));
  r->templateWidget()->setDecorationStyle(style);
  WCssTemplateRule *result = r.get();
  addRule(std::move(r), ruleName);
  return result;
}

WCssTextRule *WCssStyleSheet::addRule(const std::string& selector,
				      const WT_USTRING& declarations,
				      const std::string& ruleName)
{
  std::unique_ptr<WCssTextRule> r(new WCssTextRule(selector, declarations));
  WCssTextRule *result = r.get();
  addRule(std::move(r), ruleName);
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

std::unique_ptr<WCssRule> WCssStyleSheet::removeRule(WCssRule *rule)
{
  auto r = Utils::take(rules_, rule);

  if (r) {
    if (!Utils::erase(rulesAdded_, rule))
      rulesRemoved_.push_back(rule->selector());

    rulesModified_.erase(rule);
  }

  return r;
}

void WCssStyleSheet::ruleModified(WCssRule *rule)
{
  if (Utils::indexOf(rulesAdded_, rule) == -1)
    rulesModified_.insert(rule);
}

void WCssStyleSheet::cssText(WStringStream& out, bool all)
{
  if (all) {
    const auto& toProcess = rules_;

    for (unsigned i = 0; i < toProcess.size(); ++i) {
      auto rule = toProcess[i].get();
      out << rule->selector() << " { " << rule->declarations() << " }\n";
    }
  } else {
    const auto& toProcess = rulesAdded_;

    for (unsigned i = 0; i < toProcess.size(); ++i) {
      auto rule = toProcess[i];
      out << rule->selector() << " { " << rule->declarations() << " }\n";
    }
  }

  rulesAdded_.clear();

  if (all)
    rulesModified_.clear();
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

      DomElement *d = DomElement::updateGiven("d", DomElementType::SPAN);
      if ((*i)->updateDomElement(*d, false)) {
	EscapeOStream sout(js);
	d->asJavaScript(sout, DomElement::Priority::Update);
      }

      delete d;

      js << "}}";
    }
    rulesModified_.clear();
  }

  if (!app->environment().agentIsIElt(9)
      && app->environment().agent() != UserAgent::Konqueror) {
    if (all) {
      const auto& toProcess = rules_;

      for (unsigned i = 0; i < toProcess.size(); ++i) {
	auto rule = toProcess[i].get();
	js << WT_CLASS ".addCss('"
	   << rule->selector() << "',";
	DomElement::jsStringLiteral(js, rule->declarations(), '\'');
	js << ");\n";
      }
    } else {
      const auto& toProcess = rulesAdded_;

      for (unsigned i = 0; i < toProcess.size(); ++i) {
	auto rule = toProcess[i];
	js << WT_CLASS ".addCss('"
	   << rule->selector() << "',";
	DomElement::jsStringLiteral(js, rule->declarations(), '\'');
	js << ");\n";
      }
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

bool WCssStyleSheet::isDirty()
{
  return !rulesAdded_.empty() || !rulesModified_.empty() ||
      !rulesRemoved_.empty();
}

} // namespace Wt
