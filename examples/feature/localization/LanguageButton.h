/*
 * Copyright (C) 2025 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WPopupMenu.h>
#include <Wt/WPushButton.h>

class LanguageButton : public Wt::WPushButton
{
public:
  LanguageButton();

private:
  Wt::WPopupMenu *popup_;
  void onSelect(Wt::WMenuItem *item);
};