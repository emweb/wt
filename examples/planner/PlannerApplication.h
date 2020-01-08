// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef PLANNER_APPLICATION_H_
#define PLANNER_APPLICATION_H_

#include <Wt/WApplication.h>
#include <Wt/WString.h>

#include <Wt/Dbo/Dbo.h>

using namespace Wt;

/*
 * A planner application class which demonstrates how to customize
 * cell rendering in a calendar, paint a captcha in WPaintedWidget and
 * connect to a database via Dbo.
 */
class PlannerApplication : public WApplication
{
public:
  Dbo::Session session;

  PlannerApplication(const WEnvironment& env);

  static PlannerApplication* plannerApplication() {
    return (PlannerApplication*)WApplication::instance();
  }

private:
  void login(const Wt::WString& user);
};

#endif //PLANNER_APPLICATION_H_
