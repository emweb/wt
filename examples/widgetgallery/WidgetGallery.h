// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium
 *
 * See the LICENSE file for terms of use.
 */

#ifndef WIDGET_GALLERY_APPLICATION_H_
#define WIDGET_GALLERY_APPLICATION_H_

#include <Wt/WContainerWidget.h>

class TopicWidget;

class WidgetGallery : public Wt::WContainerWidget
{
public:
  WidgetGallery();

private:
  Wt::WNavigationBar *navigation_;
  Wt::WStackedWidget *contentsStack_;

  Wt::WMenuItem *addToMenu(Wt::WMenu *menu,
			   const Wt::WString& name,
                           std::unique_ptr<TopicWidget> topic);
};

#endif // WIDGET_GALLERY_H_
