// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba
 *
 * See the LICENSE file for terms of use.
 */

#ifndef WIDGET_GALLERY_APPLICATION_H_
#define WIDGET_GALLERY_APPLICATION_H_

#include <Wt/WContainerWidget>

namespace Wt {
  class WMenu;
  class WStackedWidget;
}

class ControlsWidget;

class WidgetGallery : public Wt::WContainerWidget
{
public:
  WidgetGallery();

private:
  Wt::WStackedWidget *contentsStack_;

  void addToMenu(Wt::WMenu *menu, const Wt::WString& name,
		 ControlsWidget *controls);
};

#endif // WIDGET_GALLERY_H_
