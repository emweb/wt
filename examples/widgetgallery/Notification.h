// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2009 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifndef NOTIFICATION_H_
#define NOTIFICATION_H_

#include "Topic.h"

#include <vector>

class Notification : public Topic
{
public:
  Notification();

  void populateSubMenu(Wt::WMenu *menu);

private:
  std::unique_ptr<Wt::WWidget> wnotification();
};

#endif // MEDIA_H_
