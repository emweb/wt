/*
 * Copyright (C) 2011 Emweb bvba, Heverlee, Belgium
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WApplication>

#include "HangmanGame.h"

Wt::WApplication *createApplication(const Wt::WEnvironment& env)
{
  Wt::WApplication *app = new Wt::WApplication(env);
  
  app->setTitle("Hangman");

  app->messageResourceBundle().use(app->appRoot() + "strings");
  app->messageResourceBundle().use(app->appRoot() + "templates");

  app->useStyleSheet("style/hangman.css");

  HangmanGame *game = new HangmanGame(app->root());

  return app;
}


int main(int argc, char **argv)
{
  return WRun(argc, argv, &createApplication);
}
