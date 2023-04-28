// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium
 *
 * See the LICENSE file for terms of use.
 */

#ifndef HANGMANGAME_H_
#define HANGMANGAME_H_

#include <Wt/WContainerWidget.h>

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
  HangmanGame();

  void handleInternalPath(const std::string &internalPath);

private:
  Wt::WStackedWidget *mainStack_ = nullptr;
  HangmanWidget *game_ = nullptr;
  HighScoresWidget *scores_ = nullptr;
  WContainerWidget *links_ = nullptr;
  Wt::WAnchor *backToGameAnchor_ = nullptr;
  Wt::WAnchor *scoresAnchor_ = nullptr;

  Session session_;

  void onAuthEvent();
  void showGame();
  void showHighScores();
};

#endif //HANGMANGAME_H_
