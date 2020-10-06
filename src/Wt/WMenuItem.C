/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WAnchor.h"
#include "Wt/WApplication.h"
#include "Wt/WCheckBox.h"
#include "Wt/WContainerWidget.h"
#include "Wt/WEnvironment.h"
#include "Wt/WException.h"
#include "Wt/WLabel.h"
#include "Wt/WMenuItem.h"
#include "Wt/WMenu.h"
#include "Wt/WPopupMenu.h"
#include "Wt/WSignal.h"
#include "Wt/WStackedWidget.h"
#include "Wt/WText.h"
#include "Wt/WTheme.h"

#include "StdWidgetItemImpl.h"

#include <algorithm>
#include <cctype>

namespace Wt {

WMenuItem::WMenuItem(const WString& text, std::unique_ptr<WWidget> contents,
		     ContentLoading policy)
  : separator_(false)
{
  create(std::string(), text, std::move(contents), policy);
}

WMenuItem::WMenuItem(const std::string& iconPath, const WString& text,
		     std::unique_ptr<WWidget> contents, ContentLoading policy)
  : separator_(false)
{
  create(iconPath, text, std::move(contents), policy);
}

WMenuItem::WMenuItem(bool separator, const WString& text)
  : separator_(true)
{
  create(std::string(), WString::Empty, nullptr, ContentLoading::Lazy);

  separator_ = separator;
  selectable_ = false;
  internalPathEnabled_ = false;

  if (!text.empty()) {
    text_ = addWidget(std::make_unique<WLabel>());
    text_->setTextFormat(TextFormat::Plain);
    text_->setText(text);
  }
}

void WMenuItem::create(const std::string& iconPath, const WString& text,
		       std::unique_ptr<WWidget> contents, ContentLoading policy)
{
  customLink_ = false;

  menu_ = nullptr;
  customPathComponent_ = false;
  internalPathEnabled_ = true;
  closeable_ = false;
  selectable_ = true;

  text_ = nullptr;
  icon_ = nullptr;
  checkBox_ = nullptr;
  subMenu_ = nullptr;
  data_ = nullptr;

  setContents(std::move(contents), policy);

  if (!separator_) {
    addWidget(std::make_unique<WAnchor>());
    updateInternalPath();
  }

  signalsConnected_ = false;

  if (!iconPath.empty())
    setIcon(iconPath);

  if (!separator_)
    setText(text);
}

WMenuItem::~WMenuItem()
{ }

void WMenuItem::setContents(std::unique_ptr<WWidget> contents,
			    ContentLoading policy)
{
  int menuIdx = -1;
  WMenu *menu = menu_;
  std::unique_ptr<WMenuItem> self;
  if (menu) {
    menuIdx = menu->indexOf(this);
    self = menu->removeItem(this);
  }

  uContents_ = std::move(contents);
  oContents_ = uContents_.get();
  loadPolicy_ = policy;

  if (uContents_ && loadPolicy_ == ContentLoading::Lazy) {
     if (!oContentsContainer_) {
       uContentsContainer_.reset(new WContainerWidget());
       oContentsContainer_ = uContentsContainer_.get();
       oContentsContainer_
         ->setJavaScriptMember("wtResize",
                               StdWidgetItemImpl::childrenResizeJS());

       oContentsContainer_->resize(WLength::Auto,
                                  WLength(100, LengthUnit::Percentage));
     }
   }

  if (menu) {
    menu->insertItem(menuIdx, std::move(self));
  }
}

bool WMenuItem::isSectionHeader() const
{
  WAnchor *a = anchor();
  return !separator_ && !a && !subMenu_ && text_;
}

WAnchor *WMenuItem::anchor() const
{
  for (int i = 0; i < count(); ++i) {
    WAnchor *result = dynamic_cast<WAnchor *>(widget(i));
    if (result)
      return result;
  }

  return nullptr;
}

void WMenuItem::setIcon(const std::string& path)
{
  if (!icon_) {
    WAnchor *a = anchor();
    if (!a)
      return;

    icon_ = a->insertWidget(0, std::make_unique<WText>(" "));

    WApplication *app = WApplication::instance();
    app->theme()->apply(this, icon_, MenuItemIcon);
  }

  icon_->decorationStyle().setBackgroundImage(WLink(path));
}

std::string WMenuItem::icon() const
{
  if (icon_)
    return icon_->decorationStyle().backgroundImage();
  else
    return std::string();
}

void WMenuItem::setText(const WString& text)
{
  if (!text_) {
    text_ = anchor()->addWidget(std::make_unique<WLabel>());
    text_->setTextFormat(TextFormat::Plain);
  }

  text_->setText(text);

  if (!customPathComponent_) {
    std::string result;
#ifdef WT_TARGET_JAVA
    WString t = text;
#else
    const WString& t = text;
#endif

    if (t.literal())
      result = t.narrow();
    else
      result = t.key();

    for (unsigned i = 0; i < result.length(); ++i) {
      if (std::isspace((unsigned char)result[i]))
	result[i] = '-';
      else if (std::isalnum((unsigned char)result[i]))
	result[i] = std::tolower((unsigned char)result[i]);
      else
	result[i] = '_';
    }

    setPathComponent(result);
    customPathComponent_ = false;
  }
}

WString WMenuItem::text() const
{
  if (text_)
    return text_->text();
  else
    return WString::Empty;
}

std::string WMenuItem::pathComponent() const
{
  return pathComponent_;
}

void WMenuItem::setInternalPathEnabled(bool enabled)
{
  internalPathEnabled_ = enabled;
  updateInternalPath();
}

bool WMenuItem::internalPathEnabled() const
{
  return internalPathEnabled_;
}

void WMenuItem::setLink(const WLink& link)
{
  WAnchor *a = anchor();
  if (a)
    a->setLink(link);

  customLink_ = true;
}

WLink WMenuItem::link() const
{
  WAnchor *a = anchor();
  if (a)
    return a->link();
  else
    return WLink();
}

void WMenuItem::updateInternalPath()
{  
  if (menu_ && menu_->internalPathEnabled() && internalPathEnabled()) {
    std::string internalPath = menu_->internalBasePath() + pathComponent();
    WLink link(LinkType::InternalPath, internalPath);
    WAnchor *a = anchor();
    if (a)
      a->setLink(link);
  } else {
    WAnchor *a = anchor();
    if (a && !customLink_) {
      if (WApplication::instance()->environment().agent() == 
	  UserAgent::IE6)
	a->setLink(WLink("#"));
      else
	a->setLink(WLink());
    }
  }
}

void WMenuItem::setPathComponent(const std::string& path)
{
  customPathComponent_ = true;
  pathComponent_ = path;

  updateInternalPath();
  if (menu_)
    menu_->itemPathChanged(this);
}

void WMenuItem::setSelectable(bool selectable)
{
  selectable_ = selectable;
}

void WMenuItem::setCloseable(bool closeable)
{
  if (closeable_ != closeable) {
    closeable_ = closeable;

    if (closeable_) {
      WText *closeIcon = insertWidget(0, std::make_unique<WText>(""));
      WApplication *app = WApplication::instance();
      app->theme()->apply(this, closeIcon, MenuItemClose);

      closeIcon->clicked().connect(this, &WMenuItem::close);
    } else
      removeWidget(widget(0));
  }
}

void WMenuItem::setCheckable(bool checkable)
{
  if (isCheckable() != checkable) {
    if (checkable) {
      checkBox_ = anchor()->insertWidget(0, std::make_unique<WCheckBox>());
      setText(text());

      text_->setBuddy(checkBox_);

      WApplication *app = WApplication::instance();
      app->theme()->apply(this, checkBox_, MenuItemCheckBox);
    } else {
      anchor()->removeWidget(checkBox_);
      checkBox_ = nullptr;
    }
  }
}

void WMenuItem::setChecked(bool checked)
{
  if (isCheckable()) {
    WCheckBox *cb = dynamic_cast<WCheckBox *>(anchor()->widget(0));
    cb->setChecked(checked);
  }
}

bool WMenuItem::isChecked() const
{
  if (isCheckable()) {
    WCheckBox *cb = dynamic_cast<WCheckBox *>(anchor()->widget(0));
    return cb->isChecked();
  } else
    return false;
}

void WMenuItem::close()
{
  if (menu_)
    menu_->close(this);
}

void WMenuItem::enableAjax()
{
  if (menu_->internalPathEnabled())
    resetLearnedSlots();

  if (uContents_)
    uContents_->enableAjax();

  WContainerWidget::enableAjax();
}

void WMenuItem::setDisabled(bool disabled)
{
  WContainerWidget::setDisabled(disabled);

  if (disabled)
    if (menu_)
      menu_->onItemHidden(menu_->indexOf(this), true);
}

void WMenuItem::setHidden(bool hidden,
			  const WAnimation& animation)
{
  WContainerWidget::setHidden(hidden, animation);

  if (hidden)
    if (menu_)
      menu_->onItemHidden(menu_->indexOf(this), true);
}

void WMenuItem::render(WFlags<RenderFlag> flags)
{
  connectSignals();

  WContainerWidget::render(flags);
}

void WMenuItem::renderSelected(bool selected)
{
  WApplication *app = WApplication::instance();

  std::string active = app->theme()->activeClass();

  if (active == "Wt-selected"){ // for CSS theme, our styles are messed up
    removeStyleClass(!selected ? "itemselected" : "item", true);
    addStyleClass(selected ? "itemselected" : "item", true);
  } else
    toggleStyleClass(active, selected, true);
}

void WMenuItem::selectNotLoaded()
{
  if (!contentsLoaded())
    select();
}

bool WMenuItem::contentsLoaded() const
{
  return oContents_ && !uContents_;
}

void WMenuItem::loadContents()
{
  if (!uContents_)
    return;
  else if (!contentsLoaded()) {
    oContentsContainer_->addWidget(std::move(uContents_));
    uContents_.reset();
    signalsConnected_ = false;
    connectSignals();
  }
}

void WMenuItem::connectSignals()
{
  if (!signalsConnected_) {
    signalsConnected_ = true;

    if (!oContents_ || contentsLoaded())
      implementStateless(&WMenuItem::selectVisual,
			 &WMenuItem::undoSelectVisual);

    WAnchor *a = anchor();

    if (a) {
      SignalBase *as;
      bool selectFromCheckbox = false;

      if (checkBox_ && !checkBox_->clicked().propagationPrevented()) {
	as = &checkBox_->changed();
	/*
	 * Because the checkbox is not a properly exposed form object,
	 * we need to relay its value ourselves
	 */
	checkBox_->checked().connect(this, &WMenuItem::setCheckBox);
	checkBox_->unChecked().connect(this, &WMenuItem::setUnCheckBox);
	selectFromCheckbox = true;
      } else
	as = &a->clicked();

      if (checkBox_)
	a->setLink(WLink());

      if (uContents_)
	as->connect(this, &WMenuItem::selectNotLoaded);
      else {
	as->connect(this, &WMenuItem::selectVisual);
	if (!selectFromCheckbox)
	  as->connect(this, &WMenuItem::select);
      }
    }
  }
}

void WMenuItem::setCheckBox()
{
  setChecked(true);
  select();
}

void WMenuItem::setUnCheckBox()
{
  setChecked(false);
  select();
}

void WMenuItem::setParentMenu(WMenu *menu)
{
  menu_ = menu;

  updateInternalPath();

  if (menu && menu->isPopup() &&
      subMenu_ && subMenu_->isPopup()) {
    subMenu_->webWidget()->setZIndex(std::max(menu->zIndex() + 1000, subMenu_->zIndex()));
  }
}

WWidget *WMenuItem::contents() const
{
  return oContents_.get();
}

WWidget *WMenuItem::contentsInStack() const
{
  if (oContentsContainer_ && !uContentsContainer_)
    return oContentsContainer_.get();
  else if (oContents_ && !uContents_)
    return oContents_.get();
  else
    return nullptr;
}

std::unique_ptr<WWidget> WMenuItem::removeContents()
{
  auto contents = oContents_.get();
  oContents_.reset();

  WWidget *c = contentsInStack();

  if (c) {
    /* Remove from stack */
    std::unique_ptr<WWidget> w = c->parent()->removeWidget(c);

    if (oContentsContainer_)
      return oContentsContainer_->removeWidget(contents);
    else
      return w;
  } else {
    auto result = std::move(uContents_);
    uContents_.reset();
    return result;
  }
}

std::unique_ptr<WWidget> WMenuItem::takeContentsForStack()
{
  if (!uContents_)
    return nullptr;
  else {
    if (loadPolicy_ == ContentLoading::Lazy) {
      std::unique_ptr<WWidget> result = std::move(uContentsContainer_);
      uContentsContainer_.reset();
      return result;
    } else {
      auto result = std::move(uContents_);
      uContents_.reset();
      return result;
    }
  }
}

void WMenuItem::returnContentsInStack(std::unique_ptr<WWidget> widget)
{
  if (oContentsContainer_) {
    if (!uContents_)
      uContents_ = oContentsContainer_->removeWidget(oContents_.get());
    oContentsContainer_ = nullptr;
  } else
    uContents_ = std::move(widget);
}

void WMenuItem::setFromInternalPath(const std::string& path)
{
  if (internalPathEnabled() &&
      menu_->contentsStack_ &&
      menu_->contentsStack_->currentWidget() != contents())
    menu_->select(menu_->indexOf(this), false);

  if (subMenu_ && subMenu_->internalPathEnabled())
    subMenu_->internalPathChanged(path);
}

void WMenuItem::select()
{
  if (menu_ && selectable_ && !isDisabled())
    menu_->select(this);
}

void WMenuItem::selectVisual()
{
  if (menu_ && selectable_)
    menu_->selectVisual(this);
}

void WMenuItem::undoSelectVisual()
{
  if (menu_ && selectable_)
    menu_->undoSelectVisual();
}

void WMenuItem::setMenu(std::unique_ptr<WMenu> menu)
{
  subMenu_ = menu.get();
  subMenu_->parentItem_ = this;

  WPopupMenu *popup = dynamic_cast<WPopupMenu *>(subMenu_);
  if (popup) {
    Wt::WApplication::instance()->removeGlobalWidget(menu.get());
  }
  addWidget(std::move(menu));

  if (subMenu_->isPopup() &&
      parentMenu() && parentMenu()->isPopup()) {
    subMenu_->webWidget()->setZIndex(std::max(parentMenu()->zIndex() + 1000, subMenu_->zIndex()));
  }

  if (popup) {
    setSelectable(false);
    popup->setButton(anchor());
    updateInternalPath();
    // WPopupMenus are hidden by default, 'show' this WPopupMenu
    // but not really, since the parent is still hidden. This fixes
    // an issue where child widgets would remain unexposed, even
    // though this submenu was open (e.g. in a submenu where items
    // are checkable)
    if (dynamic_cast<WPopupMenu*>(menu_))
      popup->show();
  }
}

void WMenuItem::setItemPadding(bool padding)
{
  if (!checkBox_ && !icon_) {
    WAnchor *a = anchor();
    if (a)
      a->toggleStyleClass("Wt-padded", padding);
  }
}

}
