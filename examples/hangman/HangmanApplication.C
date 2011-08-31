/*
 * Copyright (C) 2011 Emweb bvba, Heverlee, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "HangmanApplication.h"

#include <Wt/WLogger>
#include <Wt/WVBoxLayout>

#include "HangmanGame.h"

using namespace Wt;

HangmanApplication::HangmanApplication(const WEnvironment& env)
  : WApplication(env),
    sqlite3_(WApplication::appRoot() + "hangman.db")
{
  setTitle("Hangman");

  session.setConnection(sqlite3_);
  sqlite3_.setProperty("show-queries", "true");

  session.mapClass<User>("user");

  Dbo::Transaction transaction(session);
  try {
    session.createTables();
    session.add(new User("guest", "guest"));
    log("info") << "Database created";
  } catch (...) {
    log("info") << "Using existing database";    
  }

  transaction.commit();

  messageResourceBundle().use(appRoot() + "strings");
  messageResourceBundle().use(appRoot() + "templates");

  useStyleSheet("style/hangman.css");

  new HangmanGame(root()); 
}
