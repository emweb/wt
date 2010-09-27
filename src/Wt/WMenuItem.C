/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WAnchor"
#include "Wt/WApplication"
#include "Wt/WContainerWidget"
#include "Wt/WEnvironment"
#include "Wt/WLabel"
#include "Wt/WMenuItem"
#include "Wt/WMenu"
#include "Wt/WStackedWidget"
#include "Wt/WTableCell"
#include "Wt/WText"

#include "StdGridLayoutImpl.h"
#include "WebSession.h"

#include "WtException.h"

#include <cctype>

namespace Wt {

WMenuItem::WMenuItem(const WString& text, WWidget *contents,
		     LoadPolicy policy)
  : itemWidget_(0),
    contentsContainer_(0),
    contents_(contents),
    menu_(0),
    customPathComponent_(false),
    closeable_(false),
    disabled_(false),
    hidden_(false)
{
  setText(text);

  if (policy == PreLoading)
    // prelearn the stateless slot only if already needed.
    implementStateless(&WMenuItem::selectVisual, &WMenuItem::undoSelectVisual);
  else if (contents_) {
    contentsContainer_ = new WContainerWidget();
    contentsContainer_
      ->setJavaScriptMember("wtResize", StdGridLayoutImpl::childrenResizeJS());

    addChild(contents_);

    WT_DEBUG( contentsContainer_->setObjectName("contents-container") );
    contentsContainer_->resize(WLength::Auto,
			       WLength(100, WLength::Percentage));
  }
}

WMenuItem::~WMenuItem()
{
  if (menu_)
    menu_->removeItem(this);

  delete itemWidget_;
  delete contents();
}

WWidget *WMenuItem::createItemWidget()
{
  WAnchor *enabledLabel = 0;
  WText *disabledLabel = 0;

  if (!disabled_) {
    enabledLabel = new WAnchor();
    enabledLabel->setWordWrap(false);
  } else {
    disabledLabel = new WText("");
    disabledLabel->setWordWrap(false);
  }

  if (closeable_) {
    WText *closeIcon = new WText("");
    closeIcon->setStyleClass("Wt-closeicon");
    WContainerWidget *c = new WContainerWidget();
    c->setInline(true);
    if (enabledLabel) {
      enabledLabel->setStyleClass("label");
      c->addWidget(enabledLabel);
    } else {
      disabledLabel->setStyleClass("label");
      c->addWidget(disabledLabel);
    }

    c->addWidget(closeIcon);

    return c;
  } else if (enabledLabel)
    return enabledLabel;
  else
    return disabledLabel;
}

WWidget *WMenuItem::recreateItemWidget()
{
  delete itemWidget_;
  itemWidget_ = 0;
  return itemWidget();
}

void WMenuItem::setText(const WString& text)
{
  text_ = text;

  if (!customPathComponent_) {
    std::string result;
    if (text_.literal())
      result = text_.narrow();
    else
      result = text_.key();

    for (unsigned i = 0; i < result.length(); ++i) {
      if (std::isspace((unsigned char)result[i]))
	result[i] = '-';
      else if (std::isalnum((unsigned char)result[i]))
	result[i] = std::tolower((unsigned char)result[i]);
      else
	result[i] = '_';
    }

    pathComponent_ = result;
  }

  if (itemWidget_)
    updateItemWidget(itemWidget_);
}

std::string WMenuItem::pathComponent() const
{
  return pathComponent_;
}

void WMenuItem::setPathComponent(const std::string& path)
{
  customPathComponent_ = true;
  pathComponent_ = path;

  if (itemWidget_)
    updateItemWidget(itemWidget_);
}

void WMenuItem::setCloseable(bool closeable)
{
  closeable_ = closeable;

  if (menu_)
    menu_->recreateItem(this);
}

void WMenuItem::setHidden(bool hidden)
{
  hidden_ = hidden;

  if (menu_)
    menu_->doSetHiddenItem(this, hidden_);
}

void WMenuItem::hide()
{
  setHidden(true);
}

void WMenuItem::show()
{
  setHidden(false);
}

void WMenuItem::close()
{
  if (menu_)
    menu_->close(this);
}

WWidget *WMenuItem::itemWidget()
{
  if (!itemWidget_) {
    itemWidget_ = createItemWidget();
    updateItemWidget(itemWidget_);
    connectSignals();
  }

  return itemWidget_;
}

void WMenuItem::connectActivate()
{
  SignalBase& as = activateSignal();
  if (contentsContainer_ && contentsContainer_->count() == 0)
    // load contents (will only do something on the first activation).
    as.connect(this, &WMenuItem::selectNotLoaded);
  else {
    as.connect(this, &WMenuItem::selectVisual);
    as.connect(this, &WMenuItem::select);
  }
}

void WMenuItem::connectClose()
{
  SignalBase& cs = closeSignal();
  cs.connect(this, &WMenuItem::close);
}

void WMenuItem::enableAjax()
{
  if (!contentsLoaded())
    contents_->enableAjax();

  if (menu_->internalPathEnabled()) {
    updateItemWidget(itemWidget());
    resetLearnedSlots();
  }
}

void WMenuItem::updateItemWidget(WWidget *itemWidget)
{
  WAnchor *enabledLabel = 0;
  WText *disabledLabel = 0;

  if (closeable_) {
    WContainerWidget *c = dynamic_cast<WContainerWidget *>(itemWidget);
    if (!disabled_)
      enabledLabel = dynamic_cast<WAnchor *>(c->children()[0]);
    else
      disabledLabel = dynamic_cast<WText *>(c->children()[0]);
  } else if (!disabled_)
    enabledLabel = dynamic_cast<WAnchor *>(itemWidget);
  else
    disabledLabel = dynamic_cast<WText *>(itemWidget);

  if (enabledLabel) {
    enabledLabel->setText(text());

    std::string url;
    if (menu_ && menu_->internalPathEnabled()) {
      std::string internalPath = menu_->internalBasePath() + pathComponent();
      WApplication *app = WApplication::instance();
      if (app->environment().ajax() || app->environment().agentIsSpiderBot())
	url = app->bookmarkUrl(internalPath);
      else {
	// If no JavaScript is available, then we still add the session
	// so that when used in WAnchor it will be handled by the same
	// session.
	url = app->session()->mostRelativeUrl(internalPath);
      }
    } else
      url = "#";

    enabledLabel->setRef(url);
    enabledLabel->setToolTip(toolTip());
    enabledLabel->clicked().preventDefaultAction();
  } else {
    disabledLabel->setText(text());
    disabledLabel->setToolTip(toolTip());
  }
}

SignalBase& WMenuItem::activateSignal()
{
  WWidget *w = 0;

  if (closeable_) {
    WContainerWidget *c = dynamic_cast<WContainerWidget *>(itemWidget_);
    w = c->children()[0];
  } else
    w = itemWidget_;

  WInteractWidget *wi  = dynamic_cast<WInteractWidget *>(w->webWidget());

  if (wi)
    return wi->clicked();
  else
    throw WtException("WMenuItem::activateSignal(): "
                      "could not dynamic_cast itemWidget() or "
                      "itemWidget()->children()[0] to a WInteractWidget");
}

SignalBase& WMenuItem::closeSignal()
{
  WContainerWidget *c = dynamic_cast<WContainerWidget *>(itemWidget_);
  WInteractWidget *ci = dynamic_cast<WInteractWidget *>(c->children()[1]);

  if (ci)
    return ci->clicked();
  else
    throw WtException("WMenuItem::closeSignal(): "
                      "could not dynamic_cast itemWidget()->children()[1] "
                      "to a WInteractWidget");
}

void WMenuItem::renderSelected(bool selected)
{
  if (closeable_)
    itemWidget()->setStyleClass(selected ? "citemselected" : "citem");
  else
    itemWidget()->setStyleClass(selected ? "itemselected" : "item");
}

void WMenuItem::renderHidden(bool hidden)
{
  itemWidget()->setHidden(hidden);
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
    removeChild(contents_);
    contentsContainer_->addWidget(contents_);

    // A user should do the following himself, if he wants.
    // contents_->resize(WLength::Auto, WLength(100, WLength::Percentage));

    // now prelearn the stateless slot
    implementStateless(&WMenuItem::selectVisual, &WMenuItem::undoSelectVisual);

    connectActivate();
  }
}

void WMenuItem::setMenu(WMenu *menu)
{
  menu_ = menu;
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

  if (!contentsLoaded())
    removeChild(contents_);

  contents_ = 0;

  return result;
}

void WMenuItem::setFromInternalPath(const std::string& path)
{
  if (menu_->contentsStack_
      && menu_->contentsStack_->currentWidget() != contents())
    menu_->select(menu_->indexOf(this), false);
}

void WMenuItem::select()
{
  if (menu_)
    menu_->select(this);
}

void WMenuItem::selectVisual()
{
  if (menu_)
    menu_->selectVisual(this);
}

void WMenuItem::undoSelectVisual()
{
  if (menu_)
    menu_->undoSelectVisual();
}

void WMenuItem::setToolTip(const WString& tip)
{
  tip_ = tip;

  if (itemWidget_)
    updateItemWidget(itemWidget_);
}

void WMenuItem::setDisabled(bool disabled)
{
  disabled_ = disabled;

  if (menu_)
    menu_->recreateItem(this);
}

void WMenuItem::enable()
{
  setDisabled(false);
}

void WMenuItem::disable()
{
  setDisabled(true);
}

void WMenuItem::connectSignals()
{
  if (!disabled_) {
    if (contentsLoaded())
      implementStateless(&WMenuItem::selectVisual, &WMenuItem::undoSelectVisual);
    connectActivate();
  }

  if (closeable_)
    connectClose();
}

}
