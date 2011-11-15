// This may look like C code, but it's really -*- C++ -*-
/* 
 * Copyright (C) 2011 Emweb bvba, Heverlee, Belgium
 *
 * See the LICENSE file for terms of use.
 */

#ifndef HANGMANGAME_H_
#define HANGMANGAME_H_

#include <Wt/WContainerWidget>

#include "Session.h"

namespace Wt {
  class WStackedWidget;
  class WAnchor;
}

class HangmanWidget;
class HighScoresWidget;
class Session;

class HangmanGame : public Wt::WContainerWidget
{
public:
  HangmanGame(Wt::WContainerWidget *parent = 0);

  void handleInternalPath(const std::string &internalPath);

private:
  Wt::WStackedWidget *mainStack_;
  HangmanWidget *game_;
  HighScoresWidget *scores_;
  Wt::WContainerWidget *links_;
  Wt::WAnchor *backToGameAnchor_;
  Wt::WAnchor *scoresAnchor_;

  Session session_;

  void onAuthEvent();
  void showGame();
  void showHighScores();
};

#endif //HANGMANGAME_H_
