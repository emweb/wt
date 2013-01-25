// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2009 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifndef MEDIA_H_
#define MEDIA_H_

#include "TopicWidget.h"

#include <vector>

class Media : public TopicWidget
{
public:
  Media();

  void populateSubMenu(Wt::WMenu *menu);

private:
  Wt::WWidget *mediaPlayer();
  Wt::WWidget *sound();
  Wt::WWidget *audio();
  Wt::WWidget *video();
  Wt::WWidget *flashObject();
  Wt::WWidget *resources();
  Wt::WWidget *pdf();
};

#endif // MEDIA_H_
