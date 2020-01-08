/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WBreak.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WLineEdit.h>
#include <Wt/WLogger.h>
#include <Wt/WPushButton.h>
#include <Wt/WText.h>

#include "PlannerApplication.h"

#include "Entry.h"
#include "Login.h"
#include "PlannerCalendar.h"
#include "UserAccount.h"

#include <Wt/Dbo/backend/Sqlite3.h>

PlannerApplication::PlannerApplication(const WEnvironment& env)
  : WApplication(env)
{
  auto sqlite3 = cpp14::make_unique<Dbo::backend::Sqlite3>(WApplication::appRoot() + "planner.db");
  sqlite3->setProperty("show-queries", "true");
  session.setConnection(std::move(sqlite3));

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

  Login *login = root()->addWidget(cpp14::make_unique<Login>());
  login->loggedIn().connect(this, &PlannerApplication::login);
}

void PlannerApplication::login(const WString& user)
{
  root()->clear();

  dbo::ptr<UserAccount> ua = UserAccount::login(session, user);
  root()->addWidget(cpp14::make_unique<PlannerCalendar>(ua));
}

