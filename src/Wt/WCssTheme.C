/*
 * Copyright (C) 2012 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <vector>
#include <boost/algorithm/string.hpp>

#include "Wt/WAbstractItemView"
#include "Wt/WAbstractSpinBox"
#include "Wt/WApplication"
#include "Wt/WCalendar"
#include "Wt/WCssTheme"
#include "Wt/WDateEdit"
#include "Wt/WDialog"
#include "Wt/WEnvironment"
#include "Wt/WMessageResourceBundle"
#include "Wt/WPanel"
#include "Wt/WPopupMenu"
#include "Wt/WProgressBar"
#include "Wt/WPushButton"
#include "Wt/WSuggestionPopup"
#include "Wt/WTabWidget"

#include "DomElement.h"

#ifndef WT_DEBUG_JS
#include "js/CssThemeValidate.min.js"
#endif

namespace skeletons {
  extern const char * AuthCssTheme_xml1;
}

namespace Wt {

WCssTheme::WCssTheme(const std::string& name, WObject *parent)
  : WTheme(parent),
    name_(name)
{ }

WCssTheme::~WCssTheme()
{ }

std::vector<WCssStyleSheet> WCssTheme::styleSheets() const
{
  std::vector<WCssStyleSheet> result;

  if (!name_.empty()) {
    std::string themeDir = resourcesUrl();

    WApplication *app = WApplication::instance();

    result.push_back(WCssStyleSheet(WLink(themeDir + "wt.css")));

    if (app->environment().agentIsIElt(9))
      result.push_back(WCssStyleSheet(WLink(themeDir + "wt_ie.css")));

    if (app->environment().agent() == WEnvironment::IE6)
      result.push_back(WCssStyleSheet(WLink(themeDir + "wt_ie6.css")));
  }

  return result;
}

void WCssTheme::apply(WWidget *widget, WWidget *child, int widgetRole) const
{
  switch (widgetRole) {
  case MenuItemIconRole:
    child->addStyleClass("Wt-icon");
    break;
  case MenuItemCheckBoxRole:
    child->addStyleClass("Wt-chkbox");
    break;
  case MenuItemCloseRole:
    widget->addStyleClass("Wt-closable");
    child->addStyleClass("closeicon");
    break;

  case DialogCoverRole:
    child->setStyleClass("Wt-dialogcover");
    break;
  case DialogTitleBarRole:
    child->addStyleClass("titlebar");
    break;
  case DialogBodyRole:
    child->addStyleClass("body");
    break;
  case DialogFooterRole:
    child->addStyleClass("footer");
    break;
  case DialogCloseIconRole:
    child->addStyleClass("closeicon");
    break;

  case TableViewRowContainerRole:
    {
      WAbstractItemView *view = dynamic_cast<WAbstractItemView *>(widget);

      std::string backgroundImage;

      if (view->alternatingRowColors())
	backgroundImage = "stripes/stripe-";
      else
	backgroundImage = "no-stripes/no-stripe-";

      backgroundImage = resourcesUrl() + backgroundImage
	+ boost::lexical_cast<std::string>
	  (static_cast<int>(view->rowHeight().toPixels()))
	+ "px.gif";

      child->decorationStyle().setBackgroundImage(WLink(backgroundImage));

      break;
    }

  case DatePickerPopupRole:
    child->addStyleClass("Wt-datepicker");
    break;
  case PanelTitleBarRole:
    child->addStyleClass("titlebar");
    break;
  case PanelBodyRole:
    child->addStyleClass("body");
    break;

  case AuthWidgets:
    WApplication *app = WApplication::instance();
    app->useStyleSheet(WApplication::relativeResourcesUrl() + "form.css");
    app->builtinLocalizedStrings().useBuiltin(skeletons::AuthCssTheme_xml1);
    break;
  }
}

void WCssTheme::apply(WWidget *widget, DomElement& element, int elementRole)
  const
{
  bool creating = element.mode() == DomElement::ModeCreate;

  {
    WPopupWidget *popup = dynamic_cast<WPopupWidget *>(widget);
    if (popup)
      element.addPropertyWord(PropertyClass, "Wt-outset");
  }

  switch (element.type()) {
  case DomElement_BUTTON:
    if (creating) {
      element.addPropertyWord(PropertyClass, "Wt-btn");
      WPushButton *b = dynamic_cast<WPushButton *>(widget);
      if (b) {
	if (b->isDefault())
	  element.addPropertyWord(PropertyClass, "Wt-btn-default");

	if (!b->text().empty())
	  element.addPropertyWord(PropertyClass, "with-label");
      }
    }
    break;

  case DomElement_UL:
    if (dynamic_cast<WPopupMenu *>(widget))
      element.addPropertyWord(PropertyClass, "Wt-popupmenu Wt-outset");
    else {
      WTabWidget *tabs
	= dynamic_cast<WTabWidget *>(widget->parent()->parent());

      if (tabs)
	element.addPropertyWord(PropertyClass, "Wt-tabs");
      else {
	WSuggestionPopup *suggestions
	  = dynamic_cast<WSuggestionPopup *>(widget);

	if (suggestions)
	  element.addPropertyWord(PropertyClass, "Wt-suggest");
      }
    }
    break;

  case DomElement_LI:
    {
      WMenuItem *item = dynamic_cast<WMenuItem *>(widget);
      if (item) {
	if (item->isSeparator())
	  element.addPropertyWord(PropertyClass, "Wt-separator");
   	if (item->isSectionHeader())
	  element.addPropertyWord(PropertyClass, "Wt-sectheader");
	if (item->menu())
	  element.addPropertyWord(PropertyClass, "submenu");
      }
    }
    break;

  case DomElement_DIV:
    {
      WDialog *dialog = dynamic_cast<WDialog *>(widget);
      if (dialog) {
	element.addPropertyWord(PropertyClass, "Wt-dialog");
	return;
      }

      WPanel *panel = dynamic_cast<WPanel *>(widget);
      if (panel) {
	element.addPropertyWord(PropertyClass, "Wt-panel Wt-outset");
	return;
      }

      WProgressBar *bar = dynamic_cast<WProgressBar *>(widget);
      if (bar) {
	switch (elementRole) {
	case MainElementThemeRole:
	  element.addPropertyWord(PropertyClass, "Wt-progressbar");
	  break;
	case ProgressBarBarRole:
	  element.addPropertyWord(PropertyClass, "Wt-pgb-bar");
	  break;
	case ProgressBarLabelRole:
	  element.addPropertyWord(PropertyClass, "Wt-pgb-label");
	}
	return;
      }
    }

    break;

  case DomElement_INPUT:
    {
      WAbstractSpinBox *spinBox = dynamic_cast<WAbstractSpinBox *>(widget);
      if (spinBox) {
	element.addPropertyWord(PropertyClass, "Wt-spinbox");
	return;
      }

      WDateEdit *dateEdit = dynamic_cast<WDateEdit *>(widget);
      if (dateEdit) {
	element.addPropertyWord(PropertyClass, "Wt-dateedit");
	return;
      }
    }
    break;

  default:
    break;
  }
}

std::string WCssTheme::disabledClass() const
{
  return "Wt-disabled";
}

std::string WCssTheme::activeClass() const
{
  return "Wt-selected";
}

std::string WCssTheme::utilityCssClass(int utilityCssClassRole) const
{
  switch (utilityCssClassRole) {
  case ToolTipInner:
    return "Wt-tooltip";
  default:
    return "";
  }
}

std::string WCssTheme::name() const
{
  return name_;
}

bool WCssTheme::canStyleAnchorAsButton() const
{
  return false;
}

void WCssTheme::applyValidationStyle(WWidget *widget,
				     const Wt::WValidator::Result& validation,
				     WFlags<ValidationStyleFlag> styles) const
{
  WApplication *app = WApplication::instance();

  LOAD_JAVASCRIPT(app, "js/CssThemeValidate.js", "validate", wtjs1);
  LOAD_JAVASCRIPT(app, "js/CssThemeValidate.js", "setValidationState", wtjs2);

  if (app->environment().ajax()) {
    WStringStream js;
    js << WT_CLASS ".setValidationState(" << widget->jsRef() << ","
       << (validation.state() == WValidator::Valid ? 1 : 0) << ","
       << validation.message().jsStringLiteral() << ","
       << styles.value() << ");";

    widget->doJavaScript(js.str());
  } else {
    bool validStyle
      = (validation.state() == WValidator::Valid) && 
      (styles & ValidationValidStyle);
    bool invalidStyle
      = (validation.state() != WValidator::Valid) && 
      (styles & ValidationInvalidStyle);

    widget->toggleStyleClass("Wt-valid", validStyle);
    widget->toggleStyleClass("Wt-invalid", invalidStyle);
  }
}

bool WCssTheme::canBorderBoxElement(const DomElement& element) const
{
  return true;
}

}
