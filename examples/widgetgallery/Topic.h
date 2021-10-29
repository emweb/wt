// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2012 Emweb bv, Herent, Belgium
 *
 * See the LICENSE file for terms of use.
 */

#ifndef TOPIC_H_
#define TOPIC_H_

#include <Wt/WObject.h>
#include <Wt/WString.h>

#include <string>

namespace Wt {
  class WMenu;
}

class Topic : public Wt::WObject
{
public:
  Topic();

  virtual void populateSubMenu(Wt::WMenu *menu);

  static Wt::WString reindent(const Wt::WString& text);
};

#endif // TOPIC_H_
