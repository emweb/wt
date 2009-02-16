/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WApplication"
#include "Wt/WAnchor"
#include "Wt/WContainerWidget"
#include "Wt/WEnvironment"
#include "Wt/WLabel"
#include "Wt/WMenuItem"
#include "Wt/WMenu"
#include "Wt/WTableCell"

#include "WtException.h"

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
  }
}

WMenuItem::~WMenuItem()
{
  if (menu_)
    menu_->removeItem(this);

  if (contents_ && contents_->parent() == 0)
    delete contents_;
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
    if (text.literal())
      result = text_.narrow();
    else
      result = text_.key();

    for (unsigned i = 0; i < result.length(); ++i) {
      if (isspace((unsigned char)result[i]))
	result[i] = '-';
      else if (isalnum((unsigned char)result[i]))
	result[i] = tolower((unsigned char)result[i]);
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
    as.connectBase(SLOT(this, WMenuItem::loadContents));
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
    a->clicked.setPreventDefault(true);
  }
}

SignalBase& WMenuItem::activateSignal()
{
  WInteractWidget *wi
    = dynamic_cast<WInteractWidget *>(itemWidget_->webWidget());

  if (wi)
    return wi->clicked;
  else
    throw WtException("WMenuItem::activateSignal(): "
		      "could not dynamic_cast itemWidget() to a "
		      "WInteractWidget");
}

void WMenuItem::renderSelected(bool selected)
{
  itemWidget()->setStyleClass(selected ? "itemselected" : "item");
}

void WMenuItem::loadContents()
{
  if (contentsContainer_ && contentsContainer_->count() == 0) {
    contentsContainer_->addWidget(contents_);

    // now prelearn the stateless slot
    implementStateless(&WMenuItem::selectVisual, &WMenuItem::undoSelectVisual);

    connectActivate();
    select();
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
  contents_ = 0;

  return result;
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
