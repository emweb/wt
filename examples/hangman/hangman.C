/*
 * Copyright (C) 2005 Wim Dumon
 *
 * See the LICENSE file for terms of use.
 */
#include <Wt/WApplication>
#include "HangmanGame.h"

WApplication *createApplication(const WEnvironment& env)
{
  WApplication *app = new WApplication(env);
  app->setTitle(L"Hangman");
  new HangmanGame(app->root());  

  /*
   * The application style sheet (only for the highscore widget)
   */
  WCssDecorationStyle cellStyle;
  WBorder cellBorder;
  cellBorder.setStyle(WBorder::Solid);
  cellBorder.setWidth(WBorder::Explicit, 1);
  cellBorder.setColor(WColor(Wt::lightGray));
  cellStyle.setBorder(cellBorder);

  app->styleSheet().addRule(".highscores * TD", cellStyle);

  cellStyle.font().setVariant(WFont::SmallCaps);

  app->styleSheet().addRule(".highscoresheader", cellStyle);

  cellStyle.font().setVariant(WFont::NormalVariant);
  cellStyle.font().setStyle(WFont::Italic);
  cellStyle.font().setWeight(WFont::Bold);

  app->styleSheet().addRule(".highscoresself", cellStyle);

  return app;
}

int main(int argc, char **argv)
{
   return WRun(argc, argv, &createApplication);
}
