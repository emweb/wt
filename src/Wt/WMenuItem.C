/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WApplication"
#include "Wt/WAnchor"
#include "Wt/WApplication"
#include "Wt/WContainerWidget"
#include "Wt/WEnvironment"
#include "Wt/WLabel"
#include "Wt/WMenuItem"
#include "Wt/WMenu"
#include "Wt/WStackedWidget"
#include "Wt/WTableCell"

#include "WtException.h"

#include <cctype>

namespace Wt {

WMenuItem::WMenuItem(const WString& text, WWidget *contents,
		     LoadPolicy policy)
  : itemWidget_(0),
    contentsContainer_(0),
    contents_(contents),
    customPathComponent_(false)
{
  setText(text);

  if (policy == PreLoading)
    // prelearn the stateless slot only if already needed.
    implementStateless(&WMenuItem::selectVisual, &WMenuItem::undoSelectVisual);
  else {
    contentsContainer_ = new WContainerWidget();
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
}

WWidget *WMenuItem::createItemWidget()
{
  WAnchor *result = new WAnchor();
  result->setWordWrap(false);

  return result;
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

WWidget *WMenuItem::itemWidget()
{
  if (!itemWidget_) {
    itemWidget_ = createItemWidget();
    updateItemWidget(itemWidget_);
    connectActivate();
  }

  return itemWidget_;
}

void WMenuItem::connectActivate()
{
  SignalBase& as = activateSignal();
  if (contentsContainer_ && contentsContainer_->count() == 0)
    // load contents (will only do something on the first activation).
    as.connectBase(SLOT(this, WMenuItem::selectNotLoaded));
  else {
    as.connectBase(SLOT(this, WMenuItem::selectVisual));
    as.connectBase(SLOT(this, WMenuItem::select));
  }
}

void WMenuItem::updateItemWidget(WWidget *itemWidget)
{
  WAnchor *a = dynamic_cast<WAnchor *>(itemWidget);

  if (a) {
    a->setText(text());

    std::string url;
    if (menu_->internalPathEnabled())
      url = wApp->bookmarkUrl(menu_->internalBasePath() + pathComponent());
    else
      url = "#";

    a->setRef(url);
    a->clicked().setPreventDefault(true);
  }
}

SignalBase& WMenuItem::activateSignal()
{
  WInteractWidget *wi
    = dynamic_cast<WInteractWidget *>(itemWidget_->webWidget());

  if (wi)
    return wi->clicked();
  else
    throw WtException("WMenuItem::activateSignal(): "
		      "could not dynamic_cast itemWidget() to a "
		      "WInteractWidget");
}

void WMenuItem::renderSelected(bool selected)
{
  itemWidget()->setStyleClass(selected ? "itemselected" : "item");
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
  if (menu_->contentsStack_->currentWidget() != contents())
    menu_->select(menu_->indexOf(this), false);
}

void WMenuItem::select()
{
  menu_->select(this);
}

void WMenuItem::selectVisual()
{
  menu_->selectVisual(this);
}

void WMenuItem::undoSelectVisual()
{
  menu_->undoSelectVisual();
}

}
