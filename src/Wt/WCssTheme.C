/*
 * Copyright (C) 2012 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <vector>
#include <boost/algorithm/string.hpp>

#include "Wt/WAbstractItemView.h"
#include "Wt/WAbstractSpinBox.h"
#include "Wt/WApplication.h"
#include "Wt/WCalendar.h"
#include "Wt/WCssTheme.h"
#include "Wt/WDateEdit.h"
#include "Wt/WDialog.h"
#include "Wt/WEnvironment.h"
#include "Wt/WLinkedCssStyleSheet.h"
#include "Wt/WMessageResourceBundle.h"
#include "Wt/WPanel.h"
#include "Wt/WPopupMenu.h"
#include "Wt/WProgressBar.h"
#include "Wt/WPushButton.h"
#include "Wt/WSuggestionPopup.h"
#include "Wt/WTabWidget.h"
#include "Wt/WTimeEdit.h"

#include "DomElement.h"

#ifndef WT_DEBUG_JS
#include "js/CssThemeValidate.min.js"
#endif

namespace skeletons {
  extern const char* AuthCssTheme_xml;
}

namespace Wt {

WCssTheme::WCssTheme(const std::string& name)
  : WTheme(),
    name_(name)
{ }

WCssTheme::~WCssTheme()
{ }

std::vector<WLinkedCssStyleSheet> WCssTheme::styleSheets() const
{
  std::vector<WLinkedCssStyleSheet> result;

  if (!name_.empty()) {
    std::string themeDir = resourcesUrl();

    WApplication *app = WApplication::instance();

    result.push_back(WLinkedCssStyleSheet(WLink(themeDir + "wt.css")));

    if (app->environment().agentIsIElt(9))
      result.push_back(WLinkedCssStyleSheet(WLink(themeDir + "wt_ie.css")));

    if (app->environment().agent() == UserAgent::IE6)
      result.push_back(WLinkedCssStyleSheet(WLink(themeDir + "wt_ie6.css")));
  }

  return result;
}

void WCssTheme::apply(WWidget *widget, WWidget *child, int widgetRole) const
{
  if (!widget->isThemeStyleEnabled())
    return;

  switch (widgetRole) {
  case MenuItemIcon:
    child->addStyleClass("Wt-icon");
    break;
  case MenuItemCheckBox:
    child->addStyleClass("Wt-chkbox");
    break;
  case MenuItemClose:
    widget->addStyleClass("Wt-closable");
    child->addStyleClass("closeicon");
    break;

  case DialogCoverWidget:
    child->setStyleClass("Wt-dialogcover in");
    break;
  case DialogTitleBar:
    child->addStyleClass("titlebar");
    break;
  case DialogBody:
    child->addStyleClass("body");
    break;
  case DialogFooter:
    child->addStyleClass("footer");
    break;
  case DialogCloseIcon:
    child->addStyleClass("closeicon");
    break;

  case TableViewRowContainer:
    {
      WAbstractItemView *view = dynamic_cast<WAbstractItemView *>(widget);

      std::string backgroundImage;

      if (view->alternatingRowColors())
        backgroundImage = "stripes/stripe-";
      else
        backgroundImage = "no-stripes/no-stripe-";

      backgroundImage = resourcesUrl() + backgroundImage
        + std::to_string(static_cast<int>(view->rowHeight().toPixels()))
        + "px.gif";

      child->decorationStyle().setBackgroundImage(WLink(backgroundImage));

      break;
    }

  case DatePickerPopup:
    child->addStyleClass("Wt-datepicker");
    break;
  case DatePickerIcon:
    {
      auto icon = dynamic_cast<WImage*>(child);
      icon->setImageLink(WApplication::relativeResourcesUrl() + "date.gif");
      icon->setVerticalAlignment(AlignmentFlag::Middle);
      icon->resize(16, 16);
      break;
    }

  case PanelTitleBar:
    child->addStyleClass("titlebar");
    break;
  case PanelBody:
    child->addStyleClass("body");
    break;
  case PanelCollapseButton:
    child->setFloatSide(Side::Left);
    break;

  case AuthWidgets:
    WApplication *app = WApplication::instance();
    app->useStyleSheet(WApplication::relativeResourcesUrl() + "form.css");
    app->builtinLocalizedStrings().useBuiltin(skeletons::AuthCssTheme_xml);
    break;
  }
}

void WCssTheme::apply(WWidget *widget, DomElement& element, int elementRole)
  const
{
  bool creating = element.mode() == DomElement::Mode::Create;

  if (!widget->isThemeStyleEnabled())
    return;

  {
    WPopupWidget *popup = dynamic_cast<WPopupWidget *>(widget);
    if (popup)
      element.addPropertyWord(Property::Class, "Wt-outset");
  }

  switch (element.type()) {
  case DomElementType::BUTTON:
    if (creating) {
      element.addPropertyWord(Property::Class, "Wt-btn");
      WPushButton *b = dynamic_cast<WPushButton *>(widget);
      if (b) {
        if (b->isDefault())
          element.addPropertyWord(Property::Class, "Wt-btn-default");

        if (!b->text().empty())
          element.addPropertyWord(Property::Class, "with-label");
      }
    }
    break;

  case DomElementType::UL:
    if (dynamic_cast<WPopupMenu *>(widget))
      element.addPropertyWord(Property::Class, "Wt-popupmenu Wt-outset");
    else {
      WTabWidget *tabs
        = dynamic_cast<WTabWidget *>(widget->parent()->parent());

      if (tabs)
        element.addPropertyWord(Property::Class, "Wt-tabs");
      else {
        WSuggestionPopup *suggestions
          = dynamic_cast<WSuggestionPopup *>(widget);

        if (suggestions)
          element.addPropertyWord(Property::Class, "Wt-suggest");
      }
    }
    break;

  case DomElementType::LI:
    {
      WMenuItem *item = dynamic_cast<WMenuItem *>(widget);
      if (item) {
        if (item->isSeparator())
          element.addPropertyWord(Property::Class, "Wt-separator");
           if (item->isSectionHeader())
          element.addPropertyWord(Property::Class, "Wt-sectheader");
        if (item->menu())
          element.addPropertyWord(Property::Class, "submenu");
      }
    }
    break;

  case DomElementType::DIV:
    {
      WDialog *dialog = dynamic_cast<WDialog *>(widget);
      if (dialog) {
        element.addPropertyWord(Property::Class, "Wt-dialog");
        return;
      }

      WPanel *panel = dynamic_cast<WPanel *>(widget);
      if (panel) {
        element.addPropertyWord(Property::Class, "Wt-panel Wt-outset");
        return;
      }

      WProgressBar *bar = dynamic_cast<WProgressBar *>(widget);
      if (bar) {
        switch (elementRole) {
        case MainElement:
          element.addPropertyWord(Property::Class, "Wt-progressbar");
          break;
        case ProgressBarBar:
          element.addPropertyWord(Property::Class, "Wt-pgb-bar");
          break;
        case ProgressBarLabel:
          element.addPropertyWord(Property::Class, "Wt-pgb-label");
        }
        return;
      }
    }

    break;

  case DomElementType::INPUT:
    {
      WAbstractSpinBox *spinBox = dynamic_cast<WAbstractSpinBox *>(widget);
      if (spinBox) {
        element.addPropertyWord(Property::Class, "Wt-spinbox");
        return;
      }

      WDateEdit *dateEdit = dynamic_cast<WDateEdit *>(widget);
      if (dateEdit) {
        element.addPropertyWord(Property::Class, "Wt-dateedit");
        return;
      }

      WTimeEdit *timeEdit = dynamic_cast<WTimeEdit *>(widget);
      if (timeEdit) {
        element.addPropertyWord(Property::Class, "Wt-timeedit");
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
  case ToolTipOuter:
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
       << (validation.state() == ValidationState::Valid) << ","
       << validation.message().jsStringLiteral() << ","
       << styles.value() << ");";

    widget->doJavaScript(js.str());
  } else {
    bool validStyle
      = (validation.state() == ValidationState::Valid) &&
      styles.test(ValidationStyleFlag::ValidStyle);
    bool invalidStyle
      = (validation.state() != ValidationState::Valid) &&
      styles.test(ValidationStyleFlag::InvalidStyle);

    widget->toggleStyleClass("Wt-valid", validStyle);
    widget->toggleStyleClass("Wt-invalid", invalidStyle);
  }
}

bool WCssTheme::canBorderBoxElement(const DomElement& element) const
{
  return true;
}

}
