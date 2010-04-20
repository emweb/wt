/* this is a -*-C++-*- file
 * Copyright (C) 2005 Wim Dumon
 *
 * See the LICENSE file for terms of use.
 */

#ifndef HANGMANGAME_H_
#define HANGMANGAME_H_

#include <Wt/WTable>

namespace Wt {
  class WStackedWidget;
  class WText;
}

using namespace Wt;

#include "Dictionary.h"

class HangmanWidget;
class HighScoresWidget;
class LoginWidget;

class HangmanGame : public WTable
{
   public:
      HangmanGame(WContainerWidget *parent);

   private:
      WStackedWidget   *MainStack;
      LoginWidget      *Login;
      HangmanWidget    *Game;
      HighScoresWidget *Scores;
      WText            *BackToGameText;
      WText            *ScoresText;

      // Show the initial screen
      void doLogin();

      void play(std::wstring user, Dictionary dictionary);
      void showGame();
      void showHighScores();
};

#endif
