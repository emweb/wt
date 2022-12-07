// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium
 *
 * See the LICENSE file for terms of use.
 */

#ifndef WIDGET_GALLERY_APPLICATION_H_
#define WIDGET_GALLERY_APPLICATION_H_

#include "BaseTemplate.h"

class Topic;

class WidgetGallery : public BaseTemplate
{
public:
  WidgetGallery();

private:
  Wt::WStackedWidget *contentsStack_;
  Wt::WPushButton *openMenuButton_;
  bool menuOpen_;

  Wt::WMenuItem *addToMenu(Wt::WMenu *menu,
                           const Wt::WString& name,
                           std::unique_ptr<Topic> topic);
  void toggleMenu();
  void openMenu();
  void closeMenu();
};

#endif // WIDGET_GALLERY_H_
