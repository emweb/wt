/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <boost/lexical_cast.hpp>

#include "Wt/WApplication"
#include "Wt/WCheckBox"
#include "Wt/WContainerWidget"
#include "Wt/WCssDecorationStyle"
#include "Wt/WPopupMenu"
#include "Wt/WPopupMenuItem"
#include "Wt/WText"

namespace {
  int ICON_WIDTH = 24;
  int CHECKBOX_WIDTH = 20;
  int SUBMENU_ARROW_WIDTH = 24;
}

namespace Wt {

WPopupMenuItem::WPopupMenuItem(bool)
  : text_(0),
    checkBox_(0),
    subMenu_(0),
    data_(0),
    separator_(true),
    triggered_(this)
{
  setImplementation(impl_ = new WContainerWidget());
  impl_->setLoadLaterWhenInvisible(false);
  setStyleClass("Wt-separator");
}

WPopupMenuItem::WPopupMenuItem(const WString& text)
  : text_(0),
    checkBox_(0),
    subMenu_(0),
    data_(0),
    separator_(false),
    triggered_(this)
{
  create();

  setText(text);
}

WPopupMenuItem::WPopupMenuItem(const std::string& iconPath, const WString& text)
  : text_(0),
    checkBox_(0),
    subMenu_(0),
    data_(0),
    separator_(false),
    triggered_(this)
{
  create();

  setText(text);

  if (!iconPath.empty())
    setIcon(iconPath);
}

WPopupMenuItem::~WPopupMenuItem()
{
  delete subMenu_;
}

void WPopupMenuItem::create()
{
  setImplementation(impl_ = new WContainerWidget());

  implementStateless(&WPopupMenuItem::renderOver, &WPopupMenuItem::renderOut);
  impl_->mouseWentUp().connect(this, &WPopupMenuItem::onMouseUp);

  setStyleClass("Wt-item");
}

void WPopupMenuItem::load()
{
  WCompositeWidget::load();

  //impl_->mouseWentOver().connect(parentMenu(), &WPopupMenuItem::show);
  impl_->mouseWentOver().connect(this, &WPopupMenuItem::renderOver);
  impl_->mouseWentOver().setNotExposed();
}

void WPopupMenuItem::setDisabled(bool disabled)
{
  if (disabled)
    addStyleClass("Wt-disabled");
  else
    removeStyleClass("Wt-disabled");

  resetLearnedSlot(&WPopupMenuItem::renderOver);

  WCompositeWidget::setDisabled(disabled);
}

void WPopupMenuItem::setText(const WString& text)
{
  if (!text_) {
    text_ = new WText(impl_);
    text_->setInline(false);
    text_->setMargin(ICON_WIDTH, Left);
    text_->setMargin(3, Right);
    text_->setAttributeValue
      ("style", "padding-right: "
       + boost::lexical_cast<std::string>(SUBMENU_ARROW_WIDTH) + "px");
  }

  text_->setText(text);
}

const WString& WPopupMenuItem::text() const
{
  return text_->text();
}

void WPopupMenuItem::setIcon(const std::string& path)
{
  decorationStyle().
    setBackgroundImage(path, WCssDecorationStyle::NoRepeat, CenterY);
  setAttributeValue("style", "background-position: 3px center");
}

const std::string& WPopupMenuItem::icon()
{
  return decorationStyle().backgroundImage();
}

void WPopupMenuItem::setCheckable(bool how)
{
  if (isCheckable() != how) {
    if (how) {
      text_->setMargin(ICON_WIDTH - CHECKBOX_WIDTH, Left);
      checkBox_ = new WCheckBox();
      impl_->insertWidget(0, checkBox_);
      text_->setInline(true);
    } else {
      delete checkBox_;
      text_->setMargin(ICON_WIDTH, Left);
      text_->setInline(false);
    }
  }
}

void WPopupMenuItem::setPopupMenu(WPopupMenu *menu)
{
  delete subMenu_;
  subMenu_ = menu;

  std::string resources = WApplication::resourcesUrl();

  if (subMenu_) {
    subMenu_->webWidget()->setLoadLaterWhenInvisible(false);
    subMenu_->parentItem_ = this;
    text_->decorationStyle().
      setBackgroundImage(resources + "right-arrow.gif",
			 WCssDecorationStyle::NoRepeat, Right | CenterY);
  }
}

void WPopupMenuItem::setChecked(bool how)
{
  if (checkBox_)
    checkBox_->setChecked(how);
}

bool WPopupMenuItem::isChecked() const
{
  return checkBox_ ? checkBox_->isChecked() : false;
}

void WPopupMenuItem::renderOver()
{
  parentMenu()->renderOutAll();

  if (!isDisabled())
    renderSelected(true);
}

void WPopupMenuItem::renderOut()
{
  if (!isDisabled())
    renderSelected(false);
}

void WPopupMenuItem::renderSelected(bool selected)
{
  if (separator_)
    return;

  if (selected) {
    addStyleClass("Wt-selected", true); removeStyleClass("Wt-item", true);
  } else {
    addStyleClass("Wt-item", true); removeStyleClass("Wt-selected", true);
  }

  if (subMenu_) {
    if (selected)
      subMenu_->popupToo(this);
    else {
      subMenu_->show();
      subMenu_->hide();
    }
  }
}

void WPopupMenuItem::onMouseUp()
{
  if (isDisabled() || subMenu_)
    return;

  if (checkBox_)
    checkBox_->setChecked(!checkBox_->isChecked());

  topLevelMenu()->result_ = this;

  triggered_.emit();

  topLevelMenu()->done(this);
}

WPopupMenu *WPopupMenuItem::parentMenu()
{
  return dynamic_cast<WPopupMenu *>(parent()->parent()->parent());
}

WPopupMenu *WPopupMenuItem::topLevelMenu()
{
  return parentMenu()->topLevelMenu();
}

}
