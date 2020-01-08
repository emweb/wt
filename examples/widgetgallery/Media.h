// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2009 Emweb bv, Herent, Belgium.
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
  std::unique_ptr<Wt::WWidget> mediaPlayer();
  std::unique_ptr<Wt::WWidget> sound();
  std::unique_ptr<Wt::WWidget> audio();
  std::unique_ptr<Wt::WWidget> video();
  std::unique_ptr<Wt::WWidget> flashObject();
  std::unique_ptr<Wt::WWidget> resources();
  std::unique_ptr<Wt::WWidget> pdf();
};

#endif // MEDIA_H_
