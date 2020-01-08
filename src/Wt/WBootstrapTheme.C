/*
 * Copyright (C) 2012 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <vector>

#include "Wt/WAbstractItemView.h"
#include "Wt/WAbstractSpinBox.h"
#include "Wt/WApplication.h"
#include "Wt/WBootstrapTheme.h"
#include "Wt/WCheckBox.h"
#include "Wt/WComboBox.h"
#include "Wt/WDateEdit.h"
#include "Wt/WDatePicker.h"
#include "Wt/WLinkedCssStyleSheet.h"
#include "Wt/WTimeEdit.h"
#include "Wt/WDialog.h"
#include "Wt/WEnvironment.h"
#include "Wt/WGoogleMap.h"
#include "Wt/WInPlaceEdit.h"
#include "Wt/WLabel.h"
#include "Wt/WLineEdit.h"
#include "Wt/WLogger.h"
#include "Wt/WMenu.h"
#include "Wt/WNavigationBar.h"
#include "Wt/WPanel.h"
#include "Wt/WPopupMenu.h"
#include "Wt/WProgressBar.h"
#include "Wt/WPushButton.h"
#include "Wt/WRadioButton.h"
#include "Wt/WSuggestionPopup.h"
#include "Wt/WTabWidget.h"
#include "Wt/WText.h"

#include "WebUtils.h"

#include "DomElement.h"

#ifndef WT_DEBUG_JS
#include "js/BootstrapValidate.min.js"
#endif

namespace skeletons {
  extern const char * BootstrapTheme_xml1;
  extern const char * Bootstrap3Theme_xml1;
}

namespace {
  static const std::string btnClasses[] = {
    "btn-default",
    "btn-primary",
    "btn-success",
    "btn-info",
    "btn-warning",
    "btn-danger",
    "btn-link"
  };
}

namespace Wt {

LOGGER("WBootstrapTheme");

WBootstrapTheme::WBootstrapTheme()
  : version_(BootstrapVersion::v2),
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

std::vector<WLinkedCssStyleSheet> WBootstrapTheme::styleSheets() const
{
  std::vector<WLinkedCssStyleSheet> result;

  std::string themeDir = resourcesUrl();
  std::stringstream themeVersionDir;
  themeVersionDir << themeDir << static_cast<unsigned int>(version_) << "/";

  result.push_back(WLinkedCssStyleSheet
		   (WLink(themeVersionDir.str() + "bootstrap.css")));

  WApplication *app = WApplication::instance();
 
  if (responsive_) {
    if (version_ == BootstrapVersion::v2)
      result.push_back(WLinkedCssStyleSheet
		       (WLink(themeVersionDir.str()
			      + "bootstrap-responsive.css")));
    else if (app) {
      WString v = app->metaHeader(MetaHeaderType::Meta, "viewport");
      if (v.empty())
	app->addMetaHeader("viewport",
			   "width=device-width, initial-scale=1");
    }
  }

  result.push_back(WLinkedCssStyleSheet
		   (WLink(themeVersionDir.str() + "wt.css")));

  return result;
}

void WBootstrapTheme::apply(WWidget *widget, WWidget *child, int widgetRole)
  const
{
  if (!widget->isThemeStyleEnabled())
    return;

  switch (widgetRole) {
  case MenuItemIcon:
    child->addStyleClass("Wt-icon");
    break;
  case MenuItemCheckBox:
    child->setStyleClass("Wt-chkbox");
    ((WFormWidget *)child)->label()->addStyleClass("checkbox-inline");
    break;
  case MenuItemClose:
    {
      child->addStyleClass("close");
      WText *t = dynamic_cast<WText *>(child);
      t->setText("&times;");
      break;
    }
  case DialogContent:
    if (version_ == BootstrapVersion::v3)
      child->addStyleClass("modal-content");
    break;
  case DialogCoverWidget:
    if (version_ == BootstrapVersion::v3)
      child->addStyleClass("modal-backdrop in");
    else
      child->addStyleClass("modal-backdrop Wt-bootstrap2");
    break;
  case DialogTitleBar:
       child->addStyleClass("modal-header");
    break;
  case DialogBody:
      child->addStyleClass("modal-body");
    break;
  case DialogFooter:
    child->addStyleClass("modal-footer");
    break;
  case DialogCloseIcon:
    {
      child->addStyleClass("close");
      WText *t = dynamic_cast<WText *>(child);
      t->setText("&times;");
      break;
    }

  case TableViewRowContainer:
    {
      WAbstractItemView *view = dynamic_cast<WAbstractItemView *>(widget);
      child->toggleStyleClass("Wt-striped", view->alternatingRowColors());
      break;
    }

  case DatePickerPopup:
    child->addStyleClass("Wt-datepicker");
    break;

  case TimePickerPopup:
    child->addStyleClass("Wt-timepicker");
    break;

  case PanelTitleBar:
    child->addStyleClass(classAccordionHeading());
    break;

  case PanelCollapseButton:
  case PanelTitle:
    child->addStyleClass("accordion-toggle");
    break;

  case PanelBody:
    child->addStyleClass(classAccordionInner());
    break;
  case InPlaceEditing:
    if (version_ == BootstrapVersion::v2)
      child->addStyleClass("input-append");
    else
      child->addStyleClass("input-group");
    break;
  case NavCollapse:
    child->addStyleClass(classNavCollapse());
    break;
  case NavBrand:
    child->addStyleClass(classBrand());
    break;
  case NavbarSearch:
    child->addStyleClass(classNavbarSearch());
    break;
  case NavbarAlignLeft:
    child->addStyleClass(classNavbarLeft());
    break;
  case NavbarAlignRight:
    child->addStyleClass(classNavbarRight());
    break;
  case NavbarMenu:
    child->addStyleClass(classNavbarMenu());
    break;
  case NavbarBtn:
    child->addStyleClass(classNavbarBtn());
    break;
  }
}

void WBootstrapTheme::apply(WWidget *widget, DomElement& element,
			    int elementRole) const
{
  bool creating = element.mode() == DomElement::Mode::Create;

  if (!widget->isThemeStyleEnabled())
    return;

  {
    WPopupWidget *popup = dynamic_cast<WPopupWidget *>(widget);
    if (popup) {
      WDialog *dialog = dynamic_cast<WDialog *>(widget);
      if (!dialog)
	element.addPropertyWord(Property::Class, "dropdown-menu");
    }
  }

  switch (element.type()) {

  case DomElementType::A: {
    if (creating && dynamic_cast<WPushButton *>(widget))
      element.addPropertyWord(Property::Class, classBtn(widget));

    WPushButton *btn = dynamic_cast<WPushButton *>(widget);
    if (creating && btn && btn->isDefault())
      element.addPropertyWord(Property::Class, "btn-primary");

    if (element.getProperty(Property::Class).find("dropdown-toggle")
	!= std::string::npos) {
      WMenuItem *item = dynamic_cast<WMenuItem *>(widget->parent());
      if (!dynamic_cast<WPopupMenu *>(item->parentMenu())) {
	DomElement *b = DomElement::createNew(DomElementType::B);
	b->setProperty(Property::Class, "caret");
	element.addChild(b);
      }
    }
    break;
  }

  case DomElementType::BUTTON: {
    if (creating && !widget->hasStyleClass("list-group-item"))
      element.addPropertyWord(Property::Class, classBtn(widget));

    WPushButton *button = dynamic_cast<WPushButton *>(widget);
    if (button) {
      if (creating && button->isDefault())
	element.addPropertyWord(Property::Class, "btn-primary");

      if (button->menu() && 
	  element.properties().find(Property::InnerHTML) != 
	  element.properties().end()) {
	element.addPropertyWord(Property::InnerHTML,
				"<span class=\"caret\"></span>");
      }

      if (creating && !button->text().empty())
	element.addPropertyWord(Property::Class, "with-label");

      if (!button->link().isNull())
	LOG_ERROR("Cannot use WPushButton::setLink() after the button has "
		  "been rendered with WBootstrapTheme");
    }

    break;
  }

  case DomElementType::DIV:
    {
      WDialog *dialog = dynamic_cast<WDialog *>(widget);
      if (dialog) {
        if (version_ == BootstrapVersion::v2)
          element.addPropertyWord(Property::Class, "modal");
        else
          element.addPropertyWord(Property::Class, "modal-dialog Wt-dialog");
	return;
      }

      WPanel *panel = dynamic_cast<WPanel *>(widget);
      if (panel) {
        element.addPropertyWord(Property::Class, classAccordionGroup());
	return;
      }

      WProgressBar *bar = dynamic_cast<WProgressBar *>(widget);
      if (bar) {
	switch (elementRole) {
	case MainElement:
	  element.addPropertyWord(Property::Class, "progress");
	  break;
	case ProgressBarBar:
          element.addPropertyWord(Property::Class, classBar());
	  break;
	case ProgressBarLabel:
	  element.addPropertyWord(Property::Class, "bar-label");
	}

	return;
      }

      WGoogleMap *map = dynamic_cast<WGoogleMap *>(widget);
      if (map) {
	element.addPropertyWord(Property::Class, "Wt-googlemap");
	return;
      }

      WAbstractItemView *itemView = dynamic_cast<WAbstractItemView *>(widget);
      if (itemView) {
	element.addPropertyWord(Property::Class, "form-inline");
	return;
      }

      WNavigationBar *navBar = dynamic_cast<WNavigationBar *>(widget);
      if (navBar) {
	element.addPropertyWord(Property::Class, classNavbar());
	return;
      }
    }
    break;

  case DomElementType::LABEL:
    {
      if (elementRole == 1) {
	if (version_ == BootstrapVersion::v3) {
	  WCheckBox *cb = dynamic_cast<WCheckBox *>(widget);
	  WRadioButton *rb = nullptr;
 
	  if (cb) {
	    element.addPropertyWord(Property::Class, widget->isInline() ? 
				    "checkbox-inline" : "checkbox");
	  } else {
	    rb = dynamic_cast<WRadioButton *>(widget);
	    if (rb)
	      element.addPropertyWord(Property::Class, widget->isInline() ?
				      "radio-inline" : "radio");
	  }

	  if ((cb || rb) && !widget->isInline())
	    element.setType(DomElementType::DIV);
	} else {
	  WCheckBox *cb = dynamic_cast<WCheckBox *>(widget);
	  WRadioButton *rb = nullptr;
 
	  if (cb) {
	    element.addPropertyWord(Property::Class, "checkbox");
	  } else {
	    rb = dynamic_cast<WRadioButton *>(widget);
	    if (rb)
	      element.addPropertyWord(Property::Class, "radio");
	  }

	  if ((cb || rb) && widget->isInline())
	    element.addPropertyWord(Property::Class, "inline");
	}
      }
    }
    break;

  case DomElementType::LI:
    {
      WMenuItem *item = dynamic_cast<WMenuItem *>(widget);
      if (item) {
	if (item->isSeparator())
	  element.addPropertyWord(Property::Class, "divider");
	if (item->isSectionHeader())
	  element.addPropertyWord(Property::Class, "nav-header");
	if (item->menu()) {
	  if (dynamic_cast<WPopupMenu *>(item->parentMenu()))
	    element.addPropertyWord(Property::Class, "dropdown-submenu");
	  else
	    element.addPropertyWord(Property::Class, "dropdown");
	}
      }
    }
    break;

  case DomElementType::INPUT:
    {
      if (version_ == BootstrapVersion::v3 && formControlStyle_) {
	WAbstractToggleButton *tb
	  = dynamic_cast<WAbstractToggleButton *>(widget);
	if (!tb)
	  element.addPropertyWord(Property::Class, "form-control");
      }

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
  case DomElementType::TEXTAREA:
  case DomElementType::SELECT:
    if (version_ == BootstrapVersion::v3 && formControlStyle_)
      element.addPropertyWord(Property::Class, "form-control");

    break;
  case DomElementType::UL:
    {
      WPopupMenu *popupMenu
	= dynamic_cast<WPopupMenu *>(widget);
      if (popupMenu) {
        element.addPropertyWord(Property::Class, "dropdown-menu");

	if (popupMenu->parentItem() &&
	    dynamic_cast<WPopupMenu *>(popupMenu->parentItem()->parentMenu()))
	  element.addPropertyWord(Property::Class, "submenu");
      } else {
        WMenu *menu = dynamic_cast<WMenu *>(widget);
        if (menu) {
          element.addPropertyWord(Property::Class, "nav");

          WTabWidget *tabs
              = dynamic_cast<WTabWidget *>(menu->parent()->parent());

          if (tabs)
            element.addPropertyWord(Property::Class, "nav-tabs");
	} else {
	  WSuggestionPopup *suggestions
	    = dynamic_cast<WSuggestionPopup *>(widget);

	  if (suggestions)
	    element.addPropertyWord(Property::Class, "typeahead");
	}
      }
    }

  case DomElementType::SPAN:
    {
      WInPlaceEdit *inPlaceEdit
	= dynamic_cast<WInPlaceEdit *>(widget);
      if (inPlaceEdit)
        element.addPropertyWord(Property::Class, "Wt-in-place-edit");
      else {
	WDatePicker *picker
	  = dynamic_cast<WDatePicker *>(widget);
	if (picker)
	  element.addPropertyWord(Property::Class, "Wt-datepicker");
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
       << (validation.state() == ValidationState::Valid ? 1 : 0) << ","
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

bool WBootstrapTheme::canBorderBoxElement(const DomElement& element) const
{
  // confuses the CSS for it, see #1937
  return element.type() != DomElementType::INPUT;
}

void WBootstrapTheme::setVersion(BootstrapVersion version)
{
  version_ = version;

  if (version_ == BootstrapVersion::v3) {
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

std::string WBootstrapTheme::classBtn(WWidget *widget) const
{
  Wt::WPushButton *button = dynamic_cast<Wt::WPushButton *>(widget);
  return (version_ == BootstrapVersion::v2 || hasButtonStyleClass(widget)
          || (button && button->isDefault())) ? "btn" : "btn btn-default";
}

std::string WBootstrapTheme::classBar() const
{
  return version_ == BootstrapVersion::v2 ? "bar" : "progress-bar";
}

std::string WBootstrapTheme::classAccordion() const
{
  return version_ == BootstrapVersion::v2 ? "accordion" : "panel-group";
}

std::string WBootstrapTheme::classAccordionGroup() const
{
  return version_ == BootstrapVersion::v2 ? "accordion-group" 
    : "panel panel-default";
}

std::string WBootstrapTheme::classAccordionHeading() const
{
  return version_ == BootstrapVersion::v2 ? "accordion-heading" : "panel-heading";
}

std::string WBootstrapTheme::classAccordionBody() const
{
  return version_ == BootstrapVersion::v2 ? "accordion-body" : "panel-collapse";
}

std::string WBootstrapTheme::classAccordionInner() const
{
  return version_ == BootstrapVersion::v2 ? "accordion-inner" : "panel-body";
}

std::string WBootstrapTheme::classNavbar() const
{
  return version_ == BootstrapVersion::v2 ? "navbar" : "navbar navbar-default";
}

std::string WBootstrapTheme::classNavCollapse() const
{
  return version_ == BootstrapVersion::v2 ? "nav-collapse" : "navbar-collapse";
}

std::string WBootstrapTheme::classBrand() const
{
  return version_ == BootstrapVersion::v2 ? "brand" : "navbar-brand";
}

std::string WBootstrapTheme::classNavbarSearch() const
{
  return version_ == BootstrapVersion::v2 ? "search-query" : "navbar-search";
}

std::string WBootstrapTheme::classNavbarLeft() const
{
  return version_ == BootstrapVersion::v2 ? "pull-left" : "navbar-left";
}

std::string WBootstrapTheme::classNavbarRight() const
{
  return version_ == BootstrapVersion::v2 ? "pull-right" : "navbar-right";
}

std::string WBootstrapTheme::classNavbarBtn() const
{
  return version_ == BootstrapVersion::v2 ? "btn-navbar" : "navbar-toggle";
}

std::string WBootstrapTheme::classNavbarMenu() const
{
  return version_ == BootstrapVersion::v2 ? "navbar-nav" : "navbar-nav";
}

bool WBootstrapTheme::hasButtonStyleClass(WWidget* widget) const
{
#ifndef WT_TARGET_JAVA
  int size = sizeof(btnClasses)/sizeof(std::string);
#else
  int size = Utils::sizeofFunction(btnClasses);
#endif
  for (int i = 0; i < size; ++i) {
    if (widget->hasStyleClass(btnClasses[i]))
      return true;
  }
  return false;
}

}
