// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2011 Emweb bvba, Heverlee, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifndef HANGMAN_APPLICATION_H_
#define HANGMAN_APPLICATION_H_

#include <Wt/WApplication>
#include <Wt/WString>

#include <Wt/Dbo/Dbo>
#include <Wt/Dbo/backend/Sqlite3>

#include "User.h"
#include "Dictionary.h"

class HangmanApplication : public Wt::WApplication
{
public:
  Wt::Dbo::Session   session;
  Wt::Dbo::ptr<User> user;
  Dictionary         dictionary;

  HangmanApplication(const Wt::WEnvironment& env);

  static HangmanApplication* instance() {
    return (HangmanApplication*)WApplication::instance();
  }

private:
  Wt::Dbo::backend::Sqlite3 sqlite3_;
};

#endif //HANGMAN_APPLICATION_H_
