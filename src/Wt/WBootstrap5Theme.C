/*
 * Copyright (C) 2020 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WBootstrap5Theme.h"

#include "Wt/WAbstractItemView.h"
#include "Wt/WAbstractSpinBox.h"
#include "Wt/WApplication.h"
#include "Wt/WCheckBox.h"
#include "Wt/WDateEdit.h"
#include "Wt/WDatePicker.h"
#include "Wt/WLinkedCssStyleSheet.h"
#include "Wt/WTimeEdit.h"
#include "Wt/WDialog.h"
#include "Wt/WEnvironment.h"
#include "Wt/WGoogleMap.h"
#include "Wt/WInPlaceEdit.h"
#include "Wt/WLabel.h"
#include "Wt/WLogger.h"
#include "Wt/WMenu.h"
#include "Wt/WNavigationBar.h"
#include "Wt/WPanel.h"
#include "Wt/WPopupMenu.h"
#include "Wt/WProgressBar.h"
#include "Wt/WPushButton.h"
#include "Wt/WRadioButton.h"
#include "Wt/WSlider.h"
#include "Wt/WSuggestionPopup.h"
#include "Wt/WTabWidget.h"
#include "Wt/WText.h"
#include "Wt/WImage.h"
#include "Wt/WFileUpload.h"

#include "WebUtils.h"

#include "DomElement.h"

#include <boost/algorithm/string.hpp>

#include <vector>

#ifndef WT_DEBUG_JS
#include "js/BootstrapValidate.min.js"
#include "js/Bootstrap5Theme.min.js"
#endif

namespace skeletons {
  extern const char * BootstrapTheme_xml1;
  extern const char * Bootstrap5Theme_xml1;
}

namespace {
  const std::string btnClasses[] = {
    "btn-primary",
    "btn-secondary",
    "btn-success",
    "btn-danger",
    "btn-warning",
    "btn-info",
    "btn-light",
    "btn-dark",
    "btn-link",
    "btn-outline-primary",
    "btn-outline-secondary",
    "btn-outline-success",
    "btn-outline-danger",
    "btn-outline-warning",
    "btn-outline-info",
    "btn-outline-light",
    "btn-outline-dark",
    "btn-close",
    "navbar-toggler",
    "accordion-button"
  };
}

namespace Wt {

LOGGER("WBootstrap5Theme");

WBootstrap5Theme::WBootstrap5Theme()
{ }

WBootstrap5Theme::~WBootstrap5Theme()
{ }

void WBootstrap5Theme::init(WApplication *app) const
{
  app->builtinLocalizedStrings().useBuiltin(skeletons::BootstrapTheme_xml1);
  app->builtinLocalizedStrings().useBuiltin(skeletons::Bootstrap5Theme_xml1);
  app->require(resourcesUrl() + "js/bootstrap.bundle.min.js");
  LOAD_JAVASCRIPT(app, "js/Bootstrap5Theme.js", "theme", wtjs3);
  WString v = app->metaHeader(MetaHeaderType::Meta, "viewport");
  if (v.empty()) {
    app->addMetaHeader("viewport", "width=device-width, initial-scale=1");
  }
}

std::string WBootstrap5Theme::name() const
{
  return "bootstrap5";
}

std::string WBootstrap5Theme::resourcesUrl() const
{
  return WApplication::relativeResourcesUrl() + "themes/bootstrap/5/";
}

std::vector<WLinkedCssStyleSheet> WBootstrap5Theme::styleSheets() const
{
  std::vector<WLinkedCssStyleSheet> result;

  const std::string themeDir = resourcesUrl();

  result.push_back(WLinkedCssStyleSheet(WLink(themeDir + "css/bootstrap.min.css")));
  result.push_back(WLinkedCssStyleSheet(WLink(themeDir + "wt.css")));

  return result;
}

void WBootstrap5Theme::apply(WWidget *widget, WWidget *child, int widgetRole)
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
    ((WFormWidget *)child)->label()->addStyleClass("form-checkbox");
    break;

  case MenuItemClose:
    child->addStyleClass("close");
    ((WText *)child)->setText("&times;");
    break;

  case DialogContent:
    child->addStyleClass("modal-content");
    break;

  case DialogCoverWidget:
    child->addStyleClass("modal-backdrop in");
    child->setAttributeValue("style", "opacity:0.5");
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
    child->addStyleClass("btn-close");
    break;

  case TableViewRowContainer:
    {
      auto view = dynamic_cast<WAbstractItemView *>(widget);
      child->toggleStyleClass("Wt-striped", view->alternatingRowColors());
      break;
    }

  case DatePickerPopup:
    child->addStyleClass("Wt-datepicker");
    break;

  case DatePickerIcon:
    {
      auto icon = dynamic_cast<WImage*>(child);
      icon->setImageLink(resourcesUrl() + "calendar-date.svg");
      icon->setVerticalAlignment(AlignmentFlag::Middle);
      icon->resize(20, 20);
      icon->setMargin(5, Side::Left);
      icon->addStyleClass("Wt-datepicker-icon");
      break;
    }

  case TimePickerPopup:
    child->addStyleClass("Wt-timepicker");
    break;

  case PanelTitle: {
    auto panel = dynamic_cast<WPanel*>(widget);
    if (panel && panel->isCollapsible()) {
      child->addStyleClass("accordion-item");
    }
    break;
  }

  case PanelTitleBar: {
    auto panel = dynamic_cast<WPanel*>(widget);
    if (panel && panel->isCollapsible()) {
      child->addStyleClass("accordion-header");
      child->removeStyleClass("card-header");
    } else {
      child->addStyleClass("card-header");
    }
    break;
  }

  case PanelCollapseButton: {
    auto panel = dynamic_cast<WPanel*>(widget);
    if (panel && panel->isCollapsible()) {
      child->addStyleClass("accordion-button");
    }
    break;
  }

  case PanelBody: {
    auto panel = dynamic_cast<WPanel*>(widget);
    if (panel && panel->isCollapsible()) {
      child->addStyleClass("accordion-collapse collapse show");
    } else {
      child->addStyleClass("card-body");
    }
    break;
  }

  case PanelBodyContent: {
    auto panel = dynamic_cast<WPanel*>(widget);
    if (panel && panel->isCollapsible()) {
      child->addStyleClass("accordion-body");
    }
    break;
  }

  case InPlaceEditing:
    child->addStyleClass("input-group");
    break;

  case InPlaceEditingButton:
    child->addStyleClass("btn-outline-secondary");
    break;

  case NavCollapse:
    child->addStyleClass("navbar-collapse collapse");
    break;

  case NavBrand:
    child->addStyleClass("navbar-brand");
    break;

  case NavbarSearchForm:
    child->addStyleClass("d-flex");
    break;

  case NavbarMenu:
    child->addStyleClass("navbar-nav");
    break;

  case NavbarBtn:
    child->addStyleClass("navbar-toggler");
    break;

  case TimePickerPopupContent:
    child->addStyleClass("d-flex");
    break;

  // Test to remove old BS formatting
  default:
    if (child->hasStyleClass("form-inline")) {
      child->removeStyleClass("form-inline");
      child->addStyleClass("row");
    }
    break;
  }
}

void WBootstrap5Theme::apply(WWidget *widget, DomElement& element,
                             int elementRole) const
{
  bool creating = element.mode() == DomElement::Mode::Create;

  if (!widget->isThemeStyleEnabled())
    return;

  {
    auto popup = dynamic_cast<WPopupWidget *>(widget);
    if (popup) {
      auto dialog = dynamic_cast<WDialog *>(widget);
      if (!dialog) {
        element.addPropertyWord(Property::Class, "dropdown-menu");
      }
    }
  }

  switch (element.type()) {

  case DomElementType::A: {
    if (creating && dynamic_cast<WPushButton *>(widget))
      element.addPropertyWord(Property::Class, classBtn(widget));

    auto btn = dynamic_cast<WPushButton *>(widget);
    if (creating && btn && btn->isDefault())
      element.addPropertyWord(Property::Class, "btn-primary");

    auto item = dynamic_cast<WMenuItem *>(widget->parent());
    if (item) {
      element.addPropertyWord(Property::Class, "nav-link");
    }

    if (element.getAttribute("href").empty() &&
        element.getProperty(Property::Class).empty())
      element.addPropertyWord(Property::Class, "dropdown-item");

    break;
  }

  case DomElementType::BUTTON: {
    auto button = dynamic_cast<WPushButton *>(widget);

    if (button) {
      if (creating && button->isDefault())
        element.addPropertyWord(Property::Class, "btn btn-primary");

      else if (creating)
        element.addPropertyWord(Property::Class, classBtn(widget));

      if (!button->link().isNull())
        LOG_ERROR("Cannot use WPushButton::setLink() after the button has "
                  "been rendered with WBootstrapTheme");
    }
    break;
  }

  case DomElementType::DIV: {
    auto dialog = dynamic_cast<WDialog *>(widget);
    if (dialog) {
      element.addPropertyWord(Property::Class, "modal-dialog Wt-dialog");
      return;
    }

    auto panel = dynamic_cast<WPanel *>(widget);
    if (panel) {
      if (panel->isCollapsible())
        element.addPropertyWord(Property::Class, "accordion-item");
      else
        element.addPropertyWord(Property::Class, "card");
      return;
    }

    auto bar = dynamic_cast<WProgressBar *>(widget);
    if (bar) {
      switch (elementRole) {
      case MainElement:
        element.addPropertyWord(Property::Class, "progress");
        break;
      case ProgressBarBar:
        element.addPropertyWord(Property::Class, "progress-bar");
        break;
      }
      return;
    }

    auto map = dynamic_cast<WGoogleMap *>(widget);
    if (map) {
      element.addPropertyWord(Property::Class, "Wt-googlemap");
      return;
    }

    auto navBar = dynamic_cast<const WNavigationBar *>(widget);
    if (navBar) {
      element.addPropertyWord(Property::Class, "navbar");
      if (!hasNavbarExpandClass(navBar)) {
        element.addPropertyWord(Property::Class, "navbar-expand-lg");
      }
      return;
    }
  }
  break;

  case DomElementType::LABEL: {
    if (elementRole == ToggleButtonRole) {
      auto cb = dynamic_cast<WCheckBox *>(widget);
      WRadioButton *rb = nullptr;

      if (cb)
        element.addPropertyWord(Property::Class, "form-check");
      else {
        rb = dynamic_cast<WRadioButton *>(widget);
        if (rb)
          element.addPropertyWord(Property::Class, "form-check");
      }

      if ((cb || rb) && !widget->isInline())
        element.setType(DomElementType::DIV);
      else
        element.addPropertyWord(Property::Class, "form-check-inline");
    } else if (elementRole == FormLabel) {
      element.addPropertyWord(Property::Class, "form-file-label");
    }
  }
  break;

  case DomElementType::LI: {
    auto item = dynamic_cast<WMenuItem *>(widget);
    if (item) {
      const bool separator = item->isSeparator();
      const bool sectionHeader = item->isSectionHeader();
      if (separator)
        element.addPropertyWord(Property::Class, "dropdown-divider");
      if (!separator && !sectionHeader)
        element.addPropertyWord(Property::Class, "nav-item");
      if (item->menu()) {
        if (dynamic_cast<WPopupMenu *>(item->parentMenu()))
          element.addPropertyWord(Property::Class, "dropdown");
      }
    }
  }
  break;

  case DomElementType::INPUT: {
    if (elementRole == ToggleButtonInput) {
      element.addPropertyWord(Property::Class, "form-check-input");
      break;
    } else if (elementRole == FileUploadInput) {
      element.addPropertyWord(Property::Class, "form-control");
      break;
    }

    auto tb = dynamic_cast<WAbstractToggleButton *>(widget);
    auto sl = dynamic_cast<WSlider *>(widget);
    auto fu = dynamic_cast<WFileUpload *>(widget);
    if (!(tb || sl || fu))
      element.addPropertyWord(Property::Class, "form-control");
    else if (sl) {
      element.addPropertyWord(Property::Class, "form-range");

      if (sl->orientation() == Wt::Orientation::Vertical) {
        element.addPropertyWord(Property::Class, "vertical-slider");
      }
    }

    auto spinBox = dynamic_cast<WAbstractSpinBox *>(widget);
    if (spinBox) {
      element.addPropertyWord(Property::Class, "Wt-spinbox");
      return;
    }

    auto dateEdit = dynamic_cast<WDateEdit *>(widget);
    if (dateEdit) {
      element.addPropertyWord(Property::Class, "Wt-dateedit");
      return;
    }

    auto timeEdit = dynamic_cast<WTimeEdit *>(widget);
    if (timeEdit) {
      element.addPropertyWord(Property::Class, "Wt-timeedit");
      return;
    }
  }
  break;

  case DomElementType::SELECT:
    element.addPropertyWord(Property::Class, "form-select");
    break;

  case DomElementType::TEXTAREA:
    element.addPropertyWord(Property::Class, "form-control");
    break;

  case DomElementType::UL: {
    auto popupMenu = dynamic_cast<WPopupMenu *>(widget);
    if (popupMenu) {
      element.addPropertyWord(Property::Class, "dropdown-menu");

      if (popupMenu->parentItem() &&
          dynamic_cast<WPopupMenu *>(popupMenu->parentItem()->parentMenu()))
        element.addPropertyWord(Property::Class, "submenu");
    } else {
      auto menu = dynamic_cast<WMenu *>(widget);
      if (menu) {
        if (element.getProperty(Property::Class).find("navbar-nav") == std::string::npos)
          element.addPropertyWord(Property::Class, "nav");

        auto tabs = dynamic_cast<WTabWidget *>(menu->parent()->parent());

        if (tabs)
          element.addPropertyWord(Property::Class, "nav-tabs");
      } else {
        auto suggestions = dynamic_cast<WSuggestionPopup *>(widget);

        if (suggestions)
          element.addPropertyWord(Property::Class, "typeahead");
      }
    }
    break;
  }

  case DomElementType::SPAN: {
    if (elementRole == ToggleButtonSpan)
      element.addPropertyWord(Property::Class, "form-check-label");
    else if (elementRole == FormText)
      element.addPropertyWord(Property::Class, "form-file-text");
    else if (elementRole == FormButton)
      element.addPropertyWord(Property::Class, "form-file-button");

    auto inPlaceEdit = dynamic_cast<WInPlaceEdit *>(widget);
    if (inPlaceEdit)
      element.addPropertyWord(Property::Class, "Wt-in-place-edit");
    else {
      auto picker = dynamic_cast<WDatePicker *>(widget);
      if (picker)
        element.addPropertyWord(Property::Class, "Wt-datepicker");
    }
  }
  break;

  case DomElementType::FORM:
    if (elementRole == FileUploadForm) {
      element.addPropertyWord(Property::Class, "input-group mb-2");

      // WWebWidget will grab the style class from the DOM element and apply it to the widget.
      // If we're using progressive bootstrap that means the form-control class previously applied to
      // the input gets applied to the form when enableAjax() is called. To counteract what I think
      // is mostly a hack in WWebWidget, we sadly have to add this hack.
      widget->removeStyleClass("form-control");
    }
    break;

  default:
    break;
  }
}

void WBootstrap5Theme::setDataTarget(WWidget *widget, WWidget *target) const
{
  widget->setAttributeValue("data-bs-toggle", "collapse");
  widget->setAttributeValue("data-bs-target", WString("#{1}").arg(target->id()));
}

std::string WBootstrap5Theme::disabledClass() const
{
  return "disabled";
}

std::string WBootstrap5Theme::activeClass() const
{
  return "active";
}

std::string WBootstrap5Theme::utilityCssClass(int utilityCssClassRole) const
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

bool WBootstrap5Theme::canStyleAnchorAsButton() const
{
  return true;
}

void WBootstrap5Theme::applyValidationStyle(WWidget *widget,
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
       << styles.value() << ","
       << "'is-valid', 'is-invalid');";

    widget->doJavaScript(js.str());
  } else {
    bool validStyle
      = (validation.state() == ValidationState::Valid) &&
      styles.test(ValidationStyleFlag::ValidStyle);
    bool invalidStyle
      = (validation.state() != ValidationState::Valid) &&
      styles.test(ValidationStyleFlag::InvalidStyle);

    widget->toggleStyleClass("is-valid", validStyle);
    widget->toggleStyleClass("is-invalid", invalidStyle);
  }
}

bool WBootstrap5Theme::canBorderBoxElement(const DomElement& element) const
{
  // Irrelevant, is used for old IE versions
  return true;
}

std::string WBootstrap5Theme::classBtn(const WWidget *widget)
{
  auto button = dynamic_cast<const Wt::WPushButton *>(widget);
  return (hasButtonStyleClass(widget)
          || (button && button->isDefault())) ? "btn" : "btn btn-secondary";
}

bool WBootstrap5Theme::hasButtonStyleClass(const WWidget *widget)
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

bool WBootstrap5Theme::hasNavbarExpandClass(const WNavigationBar *navigationBar)
{
  const auto classesStr = navigationBar->styleClass().toUTF8();
  std::vector<std::string> classes;
  boost::split(classes, classesStr, boost::is_any_of(" "));
  for (const auto &c : classes) {
    if (boost::starts_with(c, "navbar-expand-")) {
      return true;
    }
  }
  return false;
}

}
