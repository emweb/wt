/*
 * Copyright (C) 2025 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WApplication.h>

#include "LanguageButton.h"

LanguageButton::LanguageButton()
{
  setText(Wt::WString::tr("language"));
  auto popupMenu = std::make_unique<Wt::WPopupMenu>();
  popup_ = popupMenu.get();
  setMenu(std::move(popupMenu));

  popup_->addItem("English");
  popup_->addItem("Français");
  popup_->addItem("Nederlands");

  popup_->itemSelected().connect(this, &LanguageButton::onSelect);
}

void LanguageButton::onSelect(Wt::WMenuItem *item)
{
  Wt::WString lang = item->text();
  if (lang == "English") {
    Wt::WApplication::instance()->setLocale("en");
  } else if (lang == "Français") {
    Wt::WApplication::instance()->setLocale("fr");
  } else if (lang == "Nederlands") {
    Wt::WApplication::instance()->setLocale("nl");
  }
}