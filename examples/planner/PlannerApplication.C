/*
 * Copyright (C) 2008 Emweb bvba, Heverlee, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "PlannerApplication.h"

#include <Wt/WBreak>
#include <Wt/WContainerWidget>
#include <Wt/WLineEdit>
#include <Wt/WPushButton>
#include <Wt/WText>

#include "Entry.h"
#include "UserAccount.h"
#include "Login.h"
#include "PlannerCalendar.h"

using namespace Wt;
using namespace Wt::Dbo;

PlannerApplication::PlannerApplication(const WEnvironment& env)
  : sqlite3("planner.db"),
    WApplication(env)
{
  session.setConnection(sqlite3);
  sqlite3.setProperty("show-queries", "true");

  session.mapClass<UserAccount>("user_account");
  session.mapClass<Entry>("entry");

  Transaction transaction(session);
  try {
    session.createTables();
  } catch (...) {}
  transaction.commit();

  messageResourceBundle().use("planner");
  messageResourceBundle().use("captcha");
  messageResourceBundle().use("calendar");

  useStyleSheet("planner.css");

  Login *login = new Login(root());
  login->loggedIn.connect(SLOT(this, PlannerApplication::login));
}

void PlannerApplication::login(const std::string& user)
{
  root()->clear();

  ptr<UserAccount> ua  = UserAccount::login(session, user);
  new PlannerCalendar(root(), ua);
}

