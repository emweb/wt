// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2010 Emweb bvba, Heverlee, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef PLANNER_APPLICATION_H_
#define PLANNER_APPLICATION_H_

#include <Wt/WApplication>
#include <Wt/WString>

#include <Wt/Dbo/Dbo>
#include <Wt/Dbo/backend/Sqlite3>

/*
 * A planner application class which demonstrates how to customize
 * cell rendering in a calendar, paint a captcha in WPaintedWidget and
 * connect to a database via Dbo.
 */
class PlannerApplication : public Wt::WApplication
{
public:
  Wt::Dbo::Session session;

  PlannerApplication(const Wt::WEnvironment& env);

  static PlannerApplication* plannerApplication() {
    return (PlannerApplication*)WApplication::instance();
  }

private:
  Wt::Dbo::backend::Sqlite3 sqlite3_;

  void login(const Wt::WString& user);
};

#endif //PLANNER_APPLICATION_H_
