/*
 * Copyright (C) 2012 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <vector>

#include "Wt/WAbstractItemView"
#include "Wt/WAbstractSpinBox"
#include "Wt/WApplication"
#include "Wt/WBootstrapTheme"
#include "Wt/WCheckBox"
#include "Wt/WDateEdit"
#include "Wt/WDatePicker"
#include "Wt/WDialog"
#include "Wt/WEnvironment"
#include "Wt/WGoogleMap"
#include "Wt/WInPlaceEdit"
#include "Wt/WLogger"
#include "Wt/WMenu"
#include "Wt/WPanel"
#include "Wt/WPopupMenu"
#include "Wt/WProgressBar"
#include "Wt/WPushButton"
#include "Wt/WRadioButton"
#include "Wt/WSuggestionPopup"
#include "Wt/WTabWidget"
#include "Wt/WText"

#include "DomElement.h"

#ifndef WT_DEBUG_JS
#include "js/BootstrapValidate.min.js"
#endif

namespace skeletons {
  extern const char * AuthBootstrapTheme_xml1;
}

namespace Wt {

LOGGER("WBootstrapTheme");

WBootstrapTheme::WBootstrapTheme(WObject *parent)
  : WTheme(parent)
{ }

WBootstrapTheme::~WBootstrapTheme()
{ }

std::string WBootstrapTheme::name() const
{
  return "bootstrap";
}

std::vector<WCssStyleSheet> WBootstrapTheme::styleSheets() const
{
  std::vector<WCssStyleSheet> result;

  std::string themeDir = resourcesUrl();

  result.push_back(WCssStyleSheet(WLink(themeDir + "bootstrap.css")));
  result.push_back(WCssStyleSheet(WLink(themeDir
					+ "bootstrap-responsive.css")));
  result.push_back(WCssStyleSheet(WLink(themeDir + "wt.css")));

  return result;
}

void WBootstrapTheme::apply(WWidget *widget, WWidget *child, int widgetRole)
  const
{
  switch (widgetRole) {
  case MenuItemIconRole:
    child->addStyleClass("Wt-icon");
    break;
  case MenuItemCheckBoxRole:
    child->addStyleClass("Wt-chkbox");
    break;
  case MenuItemCloseRole:
    {
      WText *txt = dynamic_cast<WText *>(child);
      if (txt)
        txt->setText("<button class='close'>&times;</button>");
    }
    break;
  case DialogCoverRole:
    child->addStyleClass("modal-backdrop");
    break;
  case DialogTitleBarRole:
    child->addStyleClass("modal-header");
    break;
  case DialogBodyRole:
    child->addStyleClass("modal-body");
    break;
  case DialogFooterRole:
    child->addStyleClass("modal-footer");
    break;
  case DialogCloseIconRole:
    {
      child->addStyleClass("close");
      WText *t = dynamic_cast<WText *>(child);
      t->setText("&times;");
      break;
    }

  case TableViewRowContainerRole:
    {
      WAbstractItemView *view = dynamic_cast<WAbstractItemView *>(widget);
      child->toggleStyleClass("Wt-striped", view->alternatingRowColors());
      break;
    }

  case DatePickerPopupRole:
    child->addStyleClass("Wt-datepicker");
    break;

  case PanelTitleBarRole:
    child->addStyleClass("accordion-heading");
    break;

  case PanelCollapseButtonRole:
  case PanelTitleRole:
    child->addStyleClass("accordion-toggle");
    break;

  case PanelBodyRole:
    child->addStyleClass("accordion-inner");
    break;
  case AuthWidgets:
    WApplication *app = WApplication::instance();
    app->builtinLocalizedStrings().useBuiltin
      (skeletons::AuthBootstrapTheme_xml1);
    break;
  }
}

void WBootstrapTheme::apply(WWidget *widget, DomElement& element,
			    int elementRole) const
{
  bool creating = element.mode() == DomElement::ModeCreate;

  {
    WPopupWidget *popup = dynamic_cast<WPopupWidget *>(widget);
    if (popup) {
      WDialog *dialog = dynamic_cast<WDialog *>(widget);
      if (!dialog)
	element.addPropertyWord(PropertyClass, "dropdown-menu");
    }
  }

  switch (element.type()) {

  case DomElement_A:
    if (creating && dynamic_cast<WPushButton *>(widget))
      element.addPropertyWord(PropertyClass, "btn");

    if (element.getProperty(PropertyClass).find("dropdown-toggle")
	!= std::string::npos) {
      WMenuItem *item = dynamic_cast<WMenuItem *>(widget->parent());
      if (!dynamic_cast<WPopupMenu *>(item->parentMenu())) {
	DomElement *b = DomElement::createNew(DomElement_B);
	b->setProperty(PropertyClass, "caret");
	element.addChild(b);
      }
    }
    break;

  case DomElement_BUTTON: {
    if (creating)
      element.addPropertyWord(PropertyClass, "btn");

    WPushButton *button = dynamic_cast<WPushButton *>(widget);
    if (button) {
      if (creating && button->isDefault())
	element.addPropertyWord(PropertyClass, "btn-primary");

      if (button->menu() && 
	  element.properties().find(PropertyInnerHTML) != 
	  element.properties().end()) {
	element.addPropertyWord(PropertyInnerHTML,
				"<span class=\"caret\"></span>");
      }

      if (creating && !button->text().empty())
	element.addPropertyWord(PropertyClass, "with-label");

      if (!button->link().isNull())
	LOG_ERROR("Cannot use WPushButton::setLink() after the button has "
		  "been rendered with WBootstrapTheme");
    }

    break;
  }

  case DomElement_DIV:
    {
      WDialog *dialog = dynamic_cast<WDialog *>(widget);
      if (dialog) {
	element.addPropertyWord(PropertyClass, "modal");
	return;
      }

      WPanel *panel = dynamic_cast<WPanel *>(widget);
      if (panel) {
	element.addPropertyWord(PropertyClass, "accordion-group");
	return;
      }

      WProgressBar *bar = dynamic_cast<WProgressBar *>(widget);
      if (bar) {
	switch (elementRole) {
	case MainElementThemeRole:
	  element.addPropertyWord(PropertyClass, "progress");
	  break;
	case ProgressBarBarRole:
	  element.addPropertyWord(PropertyClass, "bar");
	  break;
	case ProgressBarLabelRole:
	  element.addPropertyWord(PropertyClass, "bar-label");
	}

	return;
      }

      WGoogleMap *map = dynamic_cast<WGoogleMap *>(widget);
      if (map) {
	element.addPropertyWord(PropertyClass, "Wt-googlemap");
	return;
      }

      WAbstractItemView *itemView = dynamic_cast<WAbstractItemView *>(widget);
      if (itemView) {
	element.addPropertyWord(PropertyClass, "form-horizontal");
	return;
      }
    }
    break;

  case DomElement_LABEL:
    {
      WCheckBox *cb = dynamic_cast<WCheckBox *>(widget);
      if (cb) {
	element.addPropertyWord(PropertyClass, "checkbox");
	if (cb->isInline())
	  element.addPropertyWord(PropertyClass, "inline");
      } else {
	WRadioButton *rb = dynamic_cast<WRadioButton *>(widget);
	if (rb) {
	  element.addPropertyWord(PropertyClass, "radio");
	  if (rb->isInline())
	    element.addPropertyWord(PropertyClass, "inline");
	}
      }
    }
    break;

  case DomElement_LI:
    {
      WMenuItem *item = dynamic_cast<WMenuItem *>(widget);
      if (item) {
	if (item->isSeparator())
	  element.addPropertyWord(PropertyClass, "divider");
	if (item->isSectionHeader())
	  element.addPropertyWord(PropertyClass, "nav-header");
	if (item->menu()) {
	  if (dynamic_cast<WPopupMenu *>(item->parentMenu()))
	    element.addPropertyWord(PropertyClass, "dropdown-submenu");
	  else
	    element.addPropertyWord(PropertyClass, "dropdown");
	}
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

  case DomElement_UL:
    {
      WPopupMenu *popupMenu
	= dynamic_cast<WPopupMenu *>(widget);
      if (popupMenu) {
	element.addPropertyWord(PropertyClass, "dropdown-menu");

	if (popupMenu->parentItem() &&
	    dynamic_cast<WPopupMenu *>(popupMenu->parentItem()->parentMenu()))
	  element.addPropertyWord(PropertyClass, "submenu");
      } else {
	WMenu *menu = dynamic_cast<WMenu *>(widget);
	if (menu) {
	  element.addPropertyWord(PropertyClass, "nav");

	  WTabWidget *tabs
	    = dynamic_cast<WTabWidget *>(menu->parent()->parent());

	  if (tabs)
	    element.addPropertyWord(PropertyClass, "nav-tabs");
	} else {
	  WSuggestionPopup *suggestions
	    = dynamic_cast<WSuggestionPopup *>(widget);

	  if (suggestions)
	    element.addPropertyWord(PropertyClass, "typeahead");
	}
      }
    }

  case DomElement_SPAN:
    {
      WInPlaceEdit *inPlaceEdit
	= dynamic_cast<WInPlaceEdit *>(widget);
      if (inPlaceEdit)
	element.addPropertyWord(PropertyClass, "Wt-in-place-edit");
    }
    break;
  default:
    break;
  }
}

std::string WBootstrapTheme::disabledClass() const
{
  return "disabled";
}

std::string WBootstrapTheme::activeClass() const
{
  return "active";
}

bool WBootstrapTheme::canStyleAnchorAsButton() const
{
  return true;
}

void WBootstrapTheme
::applyValidationStyle(WWidget *widget,
		       const Wt::WValidator::Result& validation,
		       WFlags<ValidationStyleFlag> styles) const
{
  WApplication *app = WApplication::instance();

  LOAD_JAVASCRIPT(app, "js/BootstrapValidate.js", "validate", wtjs1);
  LOAD_JAVASCRIPT(app, "js/BootstrapValidate.js", "setValidationState", wtjs2);

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

bool WBootstrapTheme::canBorderBoxElement(const DomElement& element) const
{
  // confuses the CSS for it, see #1937
  return element.type() != DomElement_INPUT;
}

}
