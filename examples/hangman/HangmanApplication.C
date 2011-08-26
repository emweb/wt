/*
 * Copyright (C) 2011 Emweb bvba, Heverlee, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2011 Emweb bvba, Heverlee, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "HangmanApplication.h"

#include <Wt/WLogger>

#include "User.h"
#include "HangmanGame.h"

using namespace Wt;

HangmanApplication::HangmanApplication(const WEnvironment& env)
  : WApplication(env),
    sqlite3_(Wt::WApplication::appRoot() + "hangman.db")
{
  setTitle("Hangman");

  session.setConnection(sqlite3_);
  sqlite3_.setProperty("show-queries", "true");

  session.mapClass<User>("user");

  dbo::Transaction transaction(session);
  try {
    session.createTables();
    log("info") << "Database created";
  } catch (...) {
    log("info") << "Using existing database";    
  }

  transaction.commit();

  useStyleSheet("style/hangman.css");

  new HangmanGame(root()); 
}
