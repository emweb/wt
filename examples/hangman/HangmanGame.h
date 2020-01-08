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

using namespace Wt;

namespace Wt {
  class WStackedWidget;
  class WAnchor;
}

class HangmanWidget;
class HighScoresWidget;
class Session;

class HangmanGame : public WContainerWidget
{
public:
  HangmanGame();

  void handleInternalPath(const std::string &internalPath);

private:
  WStackedWidget    *mainStack_;
  HangmanWidget     *game_;
  HighScoresWidget  *scores_;
  WContainerWidget  *links_;
  WAnchor           *backToGameAnchor_;
  WAnchor           *scoresAnchor_;

  Session session_;

  void onAuthEvent();
  void showGame();
  void showHighScores();
};

#endif //HANGMANGAME_H_
