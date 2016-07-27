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
#include "Wt/WComboBox"
#include "Wt/WDateEdit"
#include "Wt/WDatePicker"
#include "Wt/WTimeEdit"
#include "Wt/WDialog"
#include "Wt/WEnvironment"
#include "Wt/WGoogleMap"
#include "Wt/WInPlaceEdit"
#include "Wt/WLabel"
#include "Wt/WLineEdit"
#include "Wt/WLogger"
#include "Wt/WMenu"
#include "Wt/WNavigationBar"
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
  extern const char * BootstrapTheme_xml1;
  extern const char * Bootstrap3Theme_xml1;
}

namespace Wt {

LOGGER("WBootstrapTheme");

WBootstrapTheme::WBootstrapTheme(WObject *parent)
  : WTheme(parent),
    version_(Version2),
    responsive_(false),
    formControlStyle_(true)
{ 
  WApplication *app = WApplication::instance();

  if (app)
    app->builtinLocalizedStrings().useBuiltin(skeletons::BootstrapTheme_xml1);
}

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
  std::stringstream themeVersionDir;
  themeVersionDir << themeDir << version_ << "/";

  result.push_back(WCssStyleSheet
		   (WLink(themeVersionDir.str() 
			  + "bootstrap.css")));

  WApplication *app = WApplication::instance();
 
  if (responsive_) {
    if (version_ < Version3)
      result.push_back(WCssStyleSheet
		       (WLink(themeVersionDir.str()
			      + "bootstrap-responsive.css")));
    else if (app) {
      WString v = app->metaHeader(MetaName, "viewport");
      if (v.empty())
	app->addMetaHeader("viewport",
			   "width=device-width, initial-scale=1");
    }
  }

  result.push_back(WCssStyleSheet(WLink(themeVersionDir.str() + "wt.css")));

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
    child->setStyleClass("Wt-chkbox");
    ((WFormWidget *)child)->label()->addStyleClass("checkbox-inline");
    break;
  case MenuItemCloseRole:
    {
      WText *txt = dynamic_cast<WText *>(child);
      if (txt)
        txt->setText("<button class='close'>&times;</button>");
    }
    break;
  case DialogContent:
    if (version_ == Version3)
      child->addStyleClass("modal-content");
    break;
  case DialogCoverRole:
    if (version_ == Version3)
      child->addStyleClass("modal-backdrop in");
    else
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

  case TimePickerPopupRole:
    child->addStyleClass("Wt-timepicker");
    break;

  case PanelTitleBarRole:
    child->addStyleClass(classAccordionHeading());
    break;

  case PanelCollapseButtonRole:
  case PanelTitleRole:
    child->addStyleClass("accordion-toggle");
    break;

  case PanelBodyRole:
    child->addStyleClass(classAccordionInner());
    break;
  case InPlaceEditingRole:
    if (version_ == Version2)
      child->addStyleClass("input-append");
    else
      child->addStyleClass("input-group");
    break;
  case NavCollapseRole:
    child->addStyleClass(classNavCollapse());
    break;
  case NavBrandRole:
    child->addStyleClass(classBrand());
    break;
  case NavbarSearchRole:
    child->addStyleClass(classNavbarSearch());
    break;
  case NavbarAlignLeftRole:
    child->addStyleClass(classNavbarLeft());
    break;
  case NavbarAlignRightRole:
    child->addStyleClass(classNavbarRight());
    break;
  case NavbarMenuRole:
    child->addStyleClass(classNavbarMenu());
    break;
  case  NavbarBtn:
    child->addStyleClass(classNavbarBtn());
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
      element.addPropertyWord(PropertyClass, classBtn());

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
      element.addPropertyWord(PropertyClass, classBtn());

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
        if (version_ == Version2)
          element.addPropertyWord(PropertyClass, "modal");
        else
          element.addPropertyWord(PropertyClass, "modal-dialog Wt-dialog");
	return;
      }

      WPanel *panel = dynamic_cast<WPanel *>(widget);
      if (panel) {
        element.addPropertyWord(PropertyClass, classAccordionGroup());
	return;
      }

      WProgressBar *bar = dynamic_cast<WProgressBar *>(widget);
      if (bar) {
	switch (elementRole) {
	case MainElementThemeRole:
	  element.addPropertyWord(PropertyClass, "progress");
	  break;
	case ProgressBarBarRole:
          element.addPropertyWord(PropertyClass, classBar());
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
	element.addPropertyWord(PropertyClass, "form-inline");
	return;
      }

      WNavigationBar *navBar = dynamic_cast<WNavigationBar *>(widget);
      if (navBar) {
	element.addPropertyWord(PropertyClass, classNavbar());
	return;
      }
    }
    break;

  case DomElement_LABEL:
    {
      if (elementRole == 1) {
	if (version_ == Version3) {
	  WCheckBox *cb = dynamic_cast<WCheckBox *>(widget);
	  WRadioButton *rb = 0;
 
	  if (cb) {
	    element.addPropertyWord(PropertyClass, widget->isInline() ? 
				    "checkbox-inline" : "checkbox");
	  } else {
	    rb = dynamic_cast<WRadioButton *>(widget);
	    if (rb)
	      element.addPropertyWord(PropertyClass, widget->isInline() ?
				      "radio-inline" : "radio");
	  }

	  if ((cb || rb) && !widget->isInline())
	    element.setType(DomElement_DIV);
	} else {
	  WCheckBox *cb = dynamic_cast<WCheckBox *>(widget);
	  WRadioButton *rb = 0;
 
	  if (cb) {
	    element.addPropertyWord(PropertyClass, "checkbox");
	  } else {
	    rb = dynamic_cast<WRadioButton *>(widget);
	    if (rb)
	      element.addPropertyWord(PropertyClass, "radio");
	  }

	  if ((cb || rb) && widget->isInline())
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
      if (version_ == Version3 && formControlStyle_) {
	WAbstractToggleButton *tb
	  = dynamic_cast<WAbstractToggleButton *>(widget);
	if (!tb)
	  element.addPropertyWord(PropertyClass, "form-control");
      }

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
      WTimeEdit *timeEdit = dynamic_cast<WTimeEdit *>(widget);
      if (timeEdit) {
        element.addPropertyWord(PropertyClass, "Wt-timeedit");
        return;
      }

    }
    break;
  case DomElement_TEXTAREA:
  case DomElement_SELECT:
    if (version_ == Version3 && formControlStyle_)
      element.addPropertyWord(PropertyClass, "form-control");

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
      else {
	WDatePicker *picker
	  = dynamic_cast<WDatePicker *>(widget);
	if (picker)
	  element.addPropertyWord(PropertyClass, "Wt-datepicker");
      }
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

std::string WBootstrapTheme::utilityCssClass(int utilityCssClassRole) const
{
  switch (utilityCssClassRole) {
  case ToolTipInner:
    return "tooltip-inner";
  case ToolTipOuter:
    return "tooltip fade top in";
  default:
    return "";
  }
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

void WBootstrapTheme::setVersion(Version version)
{
  version_ = version;

  if (version_ >= Version3) {
    WApplication *app = WApplication::instance();
    if (app)
      app->builtinLocalizedStrings().useBuiltin
	(skeletons::Bootstrap3Theme_xml1);
  }
}

void WBootstrapTheme::setResponsive(bool enabled)
{
  responsive_ = enabled;
}

void WBootstrapTheme::setFormControlStyleEnabled(bool enabled)
{
  formControlStyle_ = enabled;
}

std::string WBootstrapTheme::classBtn() const
{
  return version_ == Version2 ? "btn" : "btn btn-default";
}

std::string WBootstrapTheme::classBar() const
{
  return version_ == Version2 ? "bar" : "progress-bar";
}

std::string WBootstrapTheme::classAccordion() const
{
  return version_ == Version2 ? "accordion" : "panel-group";
}

std::string WBootstrapTheme::classAccordionGroup() const
{
  return version_ == Version2 ? "accordion-group" : "panel panel-default";
}

std::string WBootstrapTheme::classAccordionHeading() const
{
  return version_ == Version2 ? "accordion-heading" : "panel-heading";
}

std::string WBootstrapTheme::classAccordionBody() const
{
  return version_ == Version2 ? "accordion-body" : "panel-collapse";
}

std::string WBootstrapTheme::classAccordionInner() const
{
  return version_ == Version2 ? "accordion-inner" : "panel-body";
}

std::string WBootstrapTheme::classNavbar() const
{
  return version_ == Version2 ? "navbar" : "navbar navbar-default";
}

std::string WBootstrapTheme::classNavCollapse() const
{
  return version_ == Version2 ? "nav-collapse" : "navbar-collapse";
}

std::string WBootstrapTheme::classBrand() const
{
  return version_ == Version2 ? "brand" : "navbar-brand";
}

std::string WBootstrapTheme::classNavbarSearch() const
{
  return version_ == Version2 ? "search-query" : "navbar-search";
}

std::string WBootstrapTheme::classNavbarLeft() const
{
  return version_ == Version2 ? "pull-left" : "navbar-left";
}

std::string WBootstrapTheme::classNavbarRight() const
{
  return version_ == Version2 ? "pull-right" : "navbar-right";
}

std::string WBootstrapTheme::classNavbarBtn() const
{
  return version_ == Version2 ? "btn-navbar" : "navbar-toggle";
}

std::string WBootstrapTheme::classNavbarMenu() const
{
  return version_ == Version2 ? "navbar-nav" : "navbar-nav";
}

}
