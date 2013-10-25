/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WAnchor"
#include "Wt/WApplication"
#include "Wt/WCheckBox"
#include "Wt/WContainerWidget"
#include "Wt/WEnvironment"
#include "Wt/WException"
#include "Wt/WLabel"
#include "Wt/WMenuItem"
#include "Wt/WMenu"
#include "Wt/WPopupMenu"
#include "Wt/WStackedWidget"
#include "Wt/WText"
#include "Wt/WTheme"

#include "StdWidgetItemImpl.h"

#include <cctype>

namespace Wt {

WMenuItem::WMenuItem(const WString& text, WWidget *contents,
		     LoadPolicy policy)
  : separator_(false),
    triggered_(this)
{
  create(std::string(), text, contents, policy);
}

WMenuItem::WMenuItem(const std::string& iconPath, const WString& text,
		     WWidget *contents, LoadPolicy policy)
  : separator_(false),
    triggered_(this)
{
  create(iconPath, text, contents, policy);
}

WMenuItem::WMenuItem(bool separator, const WString& text)
  : separator_(true),
    triggered_(this)
{
  create(std::string(), WString::Empty, 0, LazyLoading);

  separator_ = separator;
  selectable_ = false;
  internalPathEnabled_ = false;

  if (!text.empty()) {
    text_ = new WLabel(this);
    text_->setTextFormat(PlainText);
    text_->setText(text);
  }
}

void WMenuItem::create(const std::string& iconPath, const WString& text,
		       WWidget *contents, LoadPolicy policy)
{
  contentsContainer_ = 0;
  contents_ = contents;
  menu_ = 0;
  customPathComponent_ = false;
  internalPathEnabled_ = true;
  closeable_ = false;
  selectable_ = true;

  text_ = 0;
  icon_ = 0;
  checkBox_ = 0;
  subMenu_ = 0;
  data_ = 0;

  if (contents_ && policy != PreLoading) {
    contentsContainer_ = new WContainerWidget();
    contentsContainer_
      ->setJavaScriptMember("wtResize", StdWidgetItemImpl::childrenResizeJS());

    contentsContainer_->resize(WLength::Auto,
			       WLength(100, WLength::Percentage));
  }

  if (!separator_) {
    WAnchor *anchor = new WAnchor(this);
    updateInternalPath();
  }

  signalsConnected_ = false;

  if (!iconPath.empty())
    setIcon(iconPath);

  if (!separator_)
    setText(text);
}

WMenuItem::~WMenuItem()
{
  if (!contentsLoaded())
    delete contents_;

  delete subMenu_;
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

  return 0;
}

void WMenuItem::setIcon(const std::string& path)
{
  if (!icon_) {
    WAnchor *a = anchor();
    if (!a)
      return;

    icon_ = new WText(" ");
    a->insertWidget(0, icon_);

    WApplication *app = WApplication::instance();
    app->theme()->apply(this, icon_, MenuItemIconRole);
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
    text_ = new WLabel(anchor());
    text_->setTextFormat(Wt::PlainText);
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

const WString& WMenuItem::text() const
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
}

WLink WMenuItem::link() const
{
  WAnchor *a = anchor();
  if (a)
    return a->link();
  else
    return std::string();
}

void WMenuItem::setLinkTarget(AnchorTarget target)
{
  WAnchor *a = anchor();
  if (a)
    a->setTarget(target);
}

AnchorTarget WMenuItem::linkTarget() const
{
  WAnchor *a = anchor();
  if (a)
    return anchor()->target();
  else
    return TargetSelf;
}

void WMenuItem::updateInternalPath()
{  
  if (menu_ && menu_->internalPathEnabled() && internalPathEnabled()) {
    std::string internalPath = menu_->internalBasePath() + pathComponent();
    WLink link(WLink::InternalPath, internalPath);
    WAnchor *a = anchor();
    if (a)
      a->setLink(link);
  } else {
    WAnchor *a = anchor();
    if (a) {
      if (WApplication::instance()->environment().agent() == WEnvironment::IE6)
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
      WText *closeIcon = new WText("");
      insertWidget(0, closeIcon);
      WApplication *app = WApplication::instance();
      app->theme()->apply(this, closeIcon, MenuItemCloseRole);

      closeIcon->clicked().connect(this, &WMenuItem::close);
    } else {
      delete widget(0);
    }
  }
}

void WMenuItem::setCheckable(bool checkable)
{
  if (isCheckable() != checkable) {
    if (checkable) {
      checkBox_ = new WCheckBox();
      anchor()->insertWidget(0, checkBox_);
      setText(text());

      text_->setBuddy(checkBox_);

      WApplication *app = WApplication::instance();
      app->theme()->apply(this, checkBox_, MenuItemCheckBoxRole);
    } else {
      delete checkBox_;
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

WWidget *WMenuItem::itemWidget()
{
  return this;
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

  WContainerWidget::enableAjax();
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
  return !contentsContainer_ || contentsContainer_->count() == 1;
}

void WMenuItem::loadContents()
{
  if (!contentsLoaded()) {
    contentsContainer_->addWidget(contents_);
    signalsConnected_ = false;
    connectSignals();
  }
}

void WMenuItem::connectSignals()
{
  if (!signalsConnected_) {
    signalsConnected_ = true;

    if (contentsLoaded())
      implementStateless(&WMenuItem::selectVisual,
			 &WMenuItem::undoSelectVisual);

    WAnchor *a = anchor();

    if (a) {
      SignalBase *as;

      if (checkBox_ && !checkBox_->clicked().propagationPrevented())
	as = &checkBox_->changed();
      else
	as = &a->clicked();

      if (checkBox_)
	a->setLink(WLink());

      if (contentsContainer_ && contentsContainer_->count() == 0)
	as->connect(this, &WMenuItem::selectNotLoaded);
      else {
	as->connect(this, &WMenuItem::selectVisual);
	as->connect(this, &WMenuItem::select);
      }
    }
  }
}

void WMenuItem::setParentMenu(WMenu *menu)
{
  menu_ = menu;

  updateInternalPath();
}

WWidget *WMenuItem::contents() const
{
  if (contentsContainer_)
    return contentsContainer_;
  else
    return contents_;
}

WWidget *WMenuItem::takeContents()
{
  WWidget *result = contents_;

  if (contentsLoaded())
    if (contentsContainer_)
      contentsContainer_->removeWidget(contents_);

  contents_ = 0;

  return result;
}

void WMenuItem::purgeContents()
{
  contentsContainer_ = 0;
  contents_ = 0;
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

void WMenuItem::setMenu(WMenu *menu)
{
  subMenu_ = menu;
  subMenu_->parentItem_ = this;

  Wt::WContainerWidget *sparent
    = dynamic_cast<Wt::WContainerWidget *>(subMenu_->parent());
  if (sparent)
    sparent->removeWidget(subMenu_);

  addWidget(subMenu_);

  WPopupMenu *popup = dynamic_cast<WPopupMenu *>(subMenu_);
  if (popup) {
    popup->setJavaScriptMember("wtNoReparent", "true");
    setSelectable(false);
    popup->setButton(anchor());
    updateInternalPath();
  }
}

void WMenuItem::setSubMenu(WMenu *menu)
{
  setMenu(menu);
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
