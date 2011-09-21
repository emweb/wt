/*
 * Copyright (C) 2011 Emweb bvba, Heverlee, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Session.h"

#include <Wt/WApplication>
#include <Wt/WLogger>

using namespace Wt;

Session::Session()
  : sqlite3_(WApplication::instance()->appRoot() + "hangman.db")
{
  session_.setConnection(sqlite3_);
  sqlite3_.setProperty("show-queries", "true");

  session_.mapClass<User>("user");

  Dbo::Transaction transaction(session_);
  try {
    session_.createTables();
    session_.add(new User("guest", "guest"));    
    WApplication::instance()->log("info") << "Database created";
  } catch (...) {
    WApplication::instance()->log("info") << "Using existing database";
  }

  transaction.commit();
}

bool Session::login(std::string name, std::string password)
{
  Dbo::Transaction transaction(session_);
  user_ = session_.find<User>().where("name = ?").bind(name);
  if (!user_)
    user_ = session_.add(new User(name, password));

  bool ok = user_->authenticate(password);
  
  transaction.commit();
  
  return ok;
}

void Session::addToScore(int s)
{
  user_.modify()->score += s;
  ++user_.modify()->gamesPlayed;
  user_.modify()->lastLogin = WDateTime::currentDateTime();
}

std::vector<User> Session::topUsers(int limit)
{
  Dbo::Transaction transaction(session_);
  Users top = session_.find<User>().orderBy("score desc").limit(20);

  std::vector<User> result;
  for (Users::const_iterator i = top.begin(); i != top.end(); ++i)
    result.push_back(**i);

  transaction.commit();

  return result;
}

int Session::findRanking(const User *user)
{
  Dbo::Transaction transaction(session_);
  
  int ranking  = 
    session_.query<int>("select distinct count(score) from user")
    .where("score > ?")
    .bind(user->score);

  transaction.commit(); 
  
  return ranking + 1;
}
