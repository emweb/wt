/*
 * Copyright (C) 2020 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WBootstrap2Theme.h"

#include "Wt/WAbstractItemView.h"
#include "Wt/WAbstractSpinBox.h"
#include "Wt/WApplication.h"
#include "Wt/WCheckBox.h"
#include "Wt/WDateEdit.h"
#include "Wt/WDatePicker.h"
#include "Wt/WDialog.h"
#include "Wt/WEnvironment.h"
#include "Wt/WFormWidget.h"
#include "Wt/WGoogleMap.h"
#include "Wt/WInPlaceEdit.h"
#include "Wt/WLabel.h"
#include "Wt/WLink.h"
#include "Wt/WLinkedCssStyleSheet.h"
#include "Wt/WLogger.h"
#include "Wt/WMenuItem.h"
#include "Wt/WNavigationBar.h"
#include "Wt/WPanel.h"
#include "Wt/WPopupMenu.h"
#include "Wt/WPopupWidget.h"
#include "Wt/WProgressBar.h"
#include "Wt/WPushButton.h"
#include "Wt/WRadioButton.h"
#include "Wt/WStringStream.h"
#include "Wt/WSuggestionPopup.h"
#include "Wt/WTabWidget.h"
#include "Wt/WText.h"
#include "Wt/WTimeEdit.h"

#include "web/DomElement.h"

#ifndef WT_DEBUG_JS
#include "js/BootstrapValidate.min.js"
#endif

namespace skeletons {
  extern const char* BootstrapTheme_xml;
}

namespace Wt {

LOGGER("WBootstrap2Theme");

WBootstrap2Theme::WBootstrap2Theme()
  : responsive_(false)
{ }

WBootstrap2Theme::~WBootstrap2Theme()
{ }

std::string WBootstrap2Theme::name() const
{
  return "bootstrap2";
}

std::string WBootstrap2Theme::resourcesUrl() const
{
  return WApplication::relativeResourcesUrl() + "themes/bootstrap/2/";
}

std::vector<WLinkedCssStyleSheet> WBootstrap2Theme::styleSheets() const
{
  std::vector<WLinkedCssStyleSheet> result;

  const std::string themeDir = resourcesUrl();

  result.push_back(WLinkedCssStyleSheet
                   (WLink(themeDir + "bootstrap.css")));

  if (responsive_) {
    result.push_back(WLinkedCssStyleSheet
                     (WLink(themeDir + "bootstrap-responsive.css")));
  }

  result.push_back(WLinkedCssStyleSheet
                   (WLink(themeDir + "wt.css")));

  return result;
}

void WBootstrap2Theme::init(WApplication *app) const
{
  app->builtinLocalizedStrings().useBuiltin(skeletons::BootstrapTheme_xml);
}

void WBootstrap2Theme::apply(WWidget *widget,
                             WWidget *child,
                             int widgetRole) const
{
  if (!widget->isThemeStyleEnabled())
    return;

  switch (widgetRole) {
  case MenuItemIcon:
    child->addStyleClass("Wt-icon");
    break;
  case MenuItemCheckBox:
    child->setStyleClass("Wt-chkbox");
    static_cast<WFormWidget *>(child)->label()->addStyleClass("checkbox-inline");
    break;
  case MenuItemClose:
    {
      child->addStyleClass("close");
      static_cast<WText*>(child)->setText("&times;");
      break;
    }
  case DialogCoverWidget:
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
      static_cast<WText *>(child)->setText("&times;");
      break;
    }
  case TableViewRowContainer:
    {
      const WAbstractItemView *view = static_cast<WAbstractItemView *>(widget);
      child->toggleStyleClass("Wt-striped", view->alternatingRowColors());
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
  case TimePickerPopup:
    child->addStyleClass("Wt-timepicker");
    break;
  case PanelTitleBar:
    child->addStyleClass("accordion-heading");
    break;
  case PanelCollapseButton:
  case PanelTitle:
    child->addStyleClass("accordion-toggle");
    break;
  case PanelBody:
    child->addStyleClass("accordion-inner");
    break;
  case InPlaceEditing:
    child->addStyleClass("input-append");
    break;
  case NavCollapse:
    child->addStyleClass("nav-collapse");
    break;
  case NavBrand:
    child->addStyleClass("brand");
    break;
  case NavbarForm:
    child->addStyleClass("navbar-form");
    break;
  case NavbarSearchForm:
    child->addStyleClass("navbar-search");
    break;
  case NavbarSearchInput:
    child->addStyleClass("search-query");
    break;
  case NavbarAlignLeft:
    child->addStyleClass("pull-left");
    break;
  case NavbarAlignRight:
    child->addStyleClass("pull-right");
    break;
  case NavbarMenu:
    child->addStyleClass("navbar-nav");
    break;
  case NavbarBtn:
    child->addStyleClass("btn-navbar");
    break;
  }
}

void WBootstrap2Theme::apply(WWidget *widget,
                             DomElement &element,
                             int elementRole) const
{
  const bool creating = element.mode() == DomElement::Mode::Create;

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
  case DomElementType::A:
    {
      WPushButton *btn = dynamic_cast<WPushButton *>(widget);
      if (creating && btn) {
        element.addPropertyWord(Property::Class, "btn");
        if (btn->isDefault())
          element.addPropertyWord(Property::Class, "btn-primary");
      }

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

  case DomElementType::BUTTON:
    {
      if (creating && !widget->hasStyleClass("list-group-item"))
        element.addPropertyWord(Property::Class, "btn");

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
        element.addPropertyWord(Property::Class, "modal");
        return;
      }

      WPanel *panel = dynamic_cast<WPanel *>(widget);
      if (panel) {
        element.addPropertyWord(Property::Class, "accordion-group");
        return;
      }

      WProgressBar *bar = dynamic_cast<WProgressBar *>(widget);
      if (bar) {
        switch (elementRole) {
        case MainElement:
          element.addPropertyWord(Property::Class, "progress");
          break;
        case ProgressBarBar:
          element.addPropertyWord(Property::Class, "bar");
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
        element.addPropertyWord(Property::Class, "navbar");
        return;
      }

      break;
    }

  case DomElementType::LABEL:
    {
      if (elementRole == ToggleButtonRole) {
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

      break;
    }

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

      break;
    }

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

      break;
    }

  case DomElementType::UL:
    {
      WPopupMenu *popupMenu = dynamic_cast<WPopupMenu *>(widget);
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

      break;
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

      break;
    }
  default:
    break;
  }
}

std::string WBootstrap2Theme::disabledClass() const
{
  return "disabled";
}

std::string WBootstrap2Theme::activeClass() const
{
  return "active";
}

std::string WBootstrap2Theme::utilityCssClass(int utilityCssClassRole) const
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

bool WBootstrap2Theme::canStyleAnchorAsButton() const
{
  return true;
}

void WBootstrap2Theme::applyValidationStyle(WWidget *widget,
                                            const Wt::WValidator::Result& validation,
                                            WFlags<ValidationStyleFlag> styles) const
{
  WApplication *app = WApplication::instance();

  LOAD_JAVASCRIPT(app, "js/BootstrapValidate.js", "validate", wtjs1);
  LOAD_JAVASCRIPT(app, "js/BootstrapValidate.js", "setValidationState", wtjs2);

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

bool WBootstrap2Theme::canBorderBoxElement(const DomElement& element) const
{
  // confuses the CSS for it, see #1937
  return element.type() != DomElementType::INPUT;
}

void WBootstrap2Theme::setResponsive(bool enabled)
{
  responsive_ = enabled;
}

}
