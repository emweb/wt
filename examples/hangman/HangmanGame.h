// This may look like C code, but it's really -*- C++ -*-
/* 
 * Copyright (C) 2011 Emweb bvba, Heverlee, Belgium
 *
 * See the LICENSE file for terms of use.
 */

#ifndef HANGMANGAME_H_
#define HANGMANGAME_H_

#include <Wt/WTable>

namespace Wt {
  class WStackedWidget;
  class WAnchor;
}

class HangmanWidget;
class HighScoresWidget;
class LoginWidget;

class HangmanGame : public Wt::WContainerWidget
{
public:
  HangmanGame(Wt::WContainerWidget *parent = 0);
  
private:
  Wt::WStackedWidget   *mainStack_;
  LoginWidget          *login_;
  HangmanWidget        *game_;
  HighScoresWidget     *scores_;
  Wt::WAnchor          *backToGameAnchor_;
  Wt::WAnchor          *scoresAnchor_;
  
  void handleInternalPath();

  void showLogin();
  
  void showGame();
  void showHighScores();
};

#endif //HANGMANGAME_H_
