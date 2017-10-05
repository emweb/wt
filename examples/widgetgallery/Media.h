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
  std::unique_ptr<WWidget> mediaPlayer();
  std::unique_ptr<WWidget> sound();
  std::unique_ptr<WWidget> audio();
  std::unique_ptr<WWidget> video();
  std::unique_ptr<WWidget> flashObject();
  std::unique_ptr<WWidget> resources();
  std::unique_ptr<WWidget> pdf();
};

#endif // MEDIA_H_
