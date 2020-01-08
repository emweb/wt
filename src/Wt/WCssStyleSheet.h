// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WCSS_STYLE_SHEET_H_
#define WCSS_STYLE_SHEET_H_

#include <vector>
#include <set>
#include <string>

#include <Wt/WBreak.h>
#include <Wt/WCssDecorationStyle.h>

namespace Wt {

class WApplication;
class WCssStyleSheet;
class WCssTemplateWidget;

/*! \class WCssRule Wt/WCssStyleSheet.h Wt/WCssStyleSheet.h
 *  \brief Abstract rule in a CSS style sheet.
 *
 * A rule presents CSS style properties that are applied to a selected
 * set of elements.
 *
 * Use WCssTemplateRule if you would like to use a widget as a
 * template for specifying (<i>and</i> updating) a style rule, using
 * the widgets style properties, or WCssTextRule if you wish to
 * directly specify the CSS declarations.
 *
 * \sa WCssStyleSheet
 *
 * \ingroup style
 */
class WT_API WCssRule : public WObject
{
public:
  /*! \brief Destructor.
   */
  virtual ~WCssRule();

  /*! \brief Sets the selector.
   *
   * \note The selector can only be changed as long as the rule hasn't
   *       been rendered.
   */
  void setSelector(const std::string& selector) { selector_ = selector; }

  /*! \brief Returns the selector.
   */
  virtual std::string selector() const { return selector_; }

  /*! \brief Returns the style sheet to which this rule belongs.
   */
  WCssStyleSheet *sheet() const { return sheet_; }

  /*! \brief Indicates that the rule has changed and needs updating
   */
  void modified();

  /*! \brief Returns the declarations.
   *
   * This is a semi-colon separated list of CSS declarations.
   */
  virtual std::string declarations() = 0;

  virtual bool updateDomElement(DomElement& cssRuleElement, bool all);

protected:
  /*! \brief Creates a new CSS rule with given selector.
   */
  WCssRule(const std::string& selector);

private:
  std::string selector_;
  WCssStyleSheet *sheet_;

  friend class WCssStyleSheet;
};

/*! \class WCssTemplateRule Wt/WCssStyleSheet.h Wt/WCssStyleSheet.h
 *  \brief A CSS rule based on a template widget.
 *
 * This is a CSS rule whose CSS style properties are defined based on
 * properties of a template widget. When modifying the template
 * widget, these changes are reflected on the CSS rule and thus all
 * widgets that have this CSS rule.
 *
 * \if cpp
 * Usage example:
 * \code
 * Wt::WCssTemplateRule *styleRule =
 *   Wt::WApplication::instance()->styleSheet().addRule(
 *     std::make_unique<Wt::WCssTemplateRule>("#" + id() + " .item"));
 *
 * styleRule->templateWidget()->resize(100, WLength::Auto);
 * styleRule->templateWidget()->decorationStyle().setCursor(Cursor::PointingHand);
 * \endcode
 * \endif
 *
 * \sa Wt::WCssStyleSheet
 *
 * \ingroup style
 */
class WT_API WCssTemplateRule : public WCssRule
{
public:
  /*! \brief Creates a CSS rule with a given selector.
   *
   * The selector should be a valid CSS selector.
   *
   * \note If you want to update the rule, then the selector should be
   * unique and not contain commas, since this is not supported by
   * Microsoft Internet Explorer.
   */
  WCssTemplateRule(const std::string& selector);

  ~WCssTemplateRule();

  /*! \brief Returns the widget that is used as a template.
   *
   * Various properties of the widget are reflected in the CSS style:
   * - size and dimensions: WWidget::resize(), WWidget::setMinimumSize(),
   * and WWidget::setMaximumSize()
   * - its position: WWidget::setPositionScheme(), WWidget::setOffsets(),
   * WWidget::setFloatSide(), WWidget::setClearSides()
   * - visibility: WWidget::hide(), WWidget::show() and WWidget::setHidden()
   * - margins: WWidget::setMargin()
   * - line height: WWidget::setLineHeight()
   * - all decoration style properties: WWidget::decorationStyle()
   *
   * When modifying one of these properties of the returned widget, the
   * rule will be updated accordingly.
   */
  WWidget *templateWidget();

  virtual std::string declarations() override;

  virtual bool updateDomElement(DomElement& cssRuleElement, bool all) override;

private:
  std::unique_ptr<WCssTemplateWidget> widget_;
};

/*! \class WCssTextRule Wt/WCssStyleSheet.h Wt/WCssStyleSheet.h
 *  \brief A CSS rule specified directly using CSS declarations
 *
 * \if cpp
 * Usage example:
 * \code
 * auto styleRule = std::make_unique<Wt::WCssTextRule>(".MyWidget .item", "width: 100px; cursor: pointer;");
 * Wt::WApplication::instance()->styleSheet().addRule(std::move(styleRule));
 * \endcode
 * \endif
 *
 * \sa WCssStyleSheet
 *
 * \ingroup style
 */
class WT_API WCssTextRule : public WCssRule
{
public:
  /*! \brief Creates a CSS rule with a given selector and declarations.
   */
  WCssTextRule(const std::string& selector, 
	       const WT_USTRING& declarations);

  virtual std::string declarations() override;

private:
  WT_USTRING declarations_;
};

/*! \class WCssStyleSheet Wt/WCssStyleSheet.h Wt/WCssStyleSheet.h
 *  \brief A CSS style sheet.
 *
 * \if cpp
 * Usage example:
 * \code
 * Wt::WApplication::instance()->styleSheet().addRule(".MyWidget .item", "width: 100px; cursor: pointer;");
 * \endcode
 * \endif
 *
 * \sa WApplication::styleSheet()
 *
 * \ingroup style
 */
class WT_API WCssStyleSheet
{
public:
  /*! \brief Creates a new (internal) style sheet.
   */
  WCssStyleSheet();

  WCssStyleSheet(const WCssStyleSheet &) = delete;
  WCssStyleSheet& operator=(const WCssStyleSheet &) = delete;

  /*! \brief Destroys a style sheet, and all rules in it.
   */
  ~WCssStyleSheet();


  /*! \brief Adds a CSS rule.
   *
   * Add a rule using the CSS selector \p selector, with CSS
   * declarations in \p declarations. These declarations must be a
   * list separated by semi-colons (;).
   *
   * Optionally, you may give a \p ruleName, which may later be
   * used to check if the rule was already defined.
   *
   * \sa isDefined()
   */
  WCssTextRule *addRule(const std::string& selector,
			const WT_USTRING& declarations,
			const std::string& ruleName = std::string());

#ifndef WT_TARGET_JAVA
  /* Interprets as UTF-8 */
  WCssTextRule *addRule(const std::string& selector,
			const std::string& declarations,
			const std::string& ruleName = std::string());

  /* Interprets as UTF-8 */
  WCssTextRule *addRule(const std::string& selector,
			const char *declarations,
			const std::string& ruleName = std::string());
#endif

  /*! \brief Adds a CSS rule.
   *
   * Add a rule using the CSS selector \p selector, with styles specified
   * in \p style.
   *
   * Optionally, you may give a \p ruleName, which may later be
   * used to check if the rule was already defined.
   *
   * \sa isDefined()
   */
  WCssTemplateRule *addRule(const std::string& selector,
			    const WCssDecorationStyle& style,
			    const std::string& ruleName = std::string());

  /*! \brief Adds a CSS rule.
   *
   * Optionally, you may give a \p ruleName, which may later be
   * used to check if the rule was already defined.
   * Note: you may not pass the same rule to 2 diffrent applications.
   *
   * \sa isDefined()
   */
  WCssRule *addRule(std::unique_ptr<WCssRule> rule,
		    const std::string& ruleName = std::string());

  template <typename Rule>
  Rule *addRule(std::unique_ptr<Rule> rule)
#ifndef WT_TARGET_JAVA
  {
    Rule *result = rule.get();
    addRule(std::unique_ptr<WCssRule>(std::move(rule)));
    return result;
  }
#else // WT_TARGET_JAVA
  ;
#endif // WT_TARGET_JAVA

  /*! \brief Returns if a rule was already defined in this style sheet.
   *
   * Returns whether a rule was added with the given \p ruleName.
   *
   * \sa addRule()
   */
  bool isDefined(const std::string& ruleName) const;

  /*! \brief Removes a rule.
   */
  std::unique_ptr<WCssRule> removeRule(WCssRule *rule);

  void ruleModified(WCssRule *rule);

  void cssText(WStringStream& out, bool all);

  void javaScriptUpdate(WApplication *app, WStringStream& js, bool all);

private:
  typedef std::vector<std::unique_ptr<WCssRule> > RuleList;
  typedef std::set<WCssRule *> RuleSet;

  RuleList rules_;
  std::vector<WCssRule *> rulesAdded_;

  RuleSet rulesModified_;
  std::vector<std::string> rulesRemoved_;

  std::set<std::string> defined_;

  bool isDirty();

  friend class WebRenderer;
};

}

#endif // WCSS_STYLE_SHEET_H_
