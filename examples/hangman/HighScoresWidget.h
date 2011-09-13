// This may look like C code, but it's really -*- C++ -*-
/* 
 * Copyright (C) 2011 Emweb bvba, Heverlee, Belgium
 *
 * See the LICENSE file for terms of use.
 */

#ifndef HIGH_SCORES_WIDGET_H_
#define HIGH_SCORES_WIDGET_H_

#include <Wt/WContainerWidget>

class Session;

class HighScoresWidget: public Wt::WContainerWidget
{
public:
  HighScoresWidget(Session *session, Wt::WContainerWidget *parent = 0);
  void update();

private:
  Session *session_;
};

#endif //HIGH_SCORES_WIDGET_H_
