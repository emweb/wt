/*
 * Copyright (C) 2008 Emweb bvba, Heverlee, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WBreak>
#include <Wt/WContainerWidget>
#include <Wt/WLineEdit>
#include <Wt/WLogger>
#include <Wt/WPushButton>
#include <Wt/WText>

#include "PlannerApplication.h"

#include "Entry.h"
#include "Login.h"
#include "PlannerCalendar.h"
#include "UserAccount.h"

using namespace Wt;

PlannerApplication::PlannerApplication(const WEnvironment& env)
  : WApplication(env),
    sqlite3_(Wt::WApplication::appRoot() + "planner.db")
{
  session.setConnection(sqlite3_);
  sqlite3_.setProperty("show-queries", "true");

  session.mapClass<UserAccount>("user_account");
  session.mapClass<Entry>("entry");

  dbo::Transaction transaction(session);
  try {
    session.createTables();
    log("info") << "Database created";
  } catch (...) {
    log("info") << "Using existing database";    
  }

  transaction.commit();

  messageResourceBundle().use(appRoot() + "planner");
  messageResourceBundle().use(appRoot() + "captcha");
  messageResourceBundle().use(appRoot() + "calendar");

  useStyleSheet("planner.css");

  Login *login = new Login(root());
  login->loggedIn().connect(this, &PlannerApplication::login);
}

void PlannerApplication::login(const WString& user)
{
  root()->clear();

  dbo::ptr<UserAccount> ua = UserAccount::login(session, user);
  new PlannerCalendar(root(), ua);
}

