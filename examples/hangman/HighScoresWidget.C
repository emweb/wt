/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WText.h>
#include <Wt/WTable.h>
#include <Wt/Dbo/Dbo.h>
#include <Wt/WAny.h>

#include "HighScoresWidget.h"
#include "Session.h"

using namespace Wt;

HighScoresWidget::HighScoresWidget(Session *session)
  : session_(session)
{
  setContentAlignment(AlignmentFlag::Center);
  setStyleClass("highscores");
}

void HighScoresWidget::update()
{
  clear();

  addNew<Wt::WText>("<h2>Hall of fame</h2>");

  int ranking = session_->findRanking();

  std::string yourScore;
  if (ranking == 1)
    yourScore = "Congratulations! You are currently leading the pack.";
  else {
    yourScore = "You are currently ranked number "
      + asString(ranking).toUTF8()
      + ". Almost there !";
  }

  WText *score = addNew<WText>("<p>" + yourScore + "</p>");
  score->addStyleClass("score");

  std::vector<User> top = session_->topUsers(20);

  WTable *table = addNew<WTable>();

  table->elementAt(0, 0)->addNew<WText>("Rank");
  table->elementAt(0, 1)->addNew<WText>("User");
  table->elementAt(0, 2)->addNew<WText>("Games");
  table->elementAt(0, 3)->addNew<WText>("Score");
  table->elementAt(0, 4)->addNew<WText>("Last game");
  table->setHeaderCount(1);

  int formerScore = -1;
  int rank = 0;
  for (auto& user : top) {

    if (user.score != formerScore) {
      formerScore = user.score;
      ++rank;
    }

    int row = table->rowCount();
    table->elementAt(row, 0)->addNew<WText>(asString(rank));
    table->elementAt(row, 1)->addNew<WText>(user.name);
    table->elementAt(row, 2)->addNew<WText>(asString(user.gamesPlayed));
    table->elementAt(row, 3)->addNew<WText>(asString(user.score));
    if (!user.lastGame.isNull())
      table->elementAt(row, 4)->addNew<WText>(user.lastGame.timeTo(WDateTime::currentDateTime()) + " ago");
    else
      table->elementAt(row, 4)->addNew<WText>("---");

    if (session_->login().loggedIn() && session_->userName() == user.name)
      table->rowAt(row)->setId("self");
  }

  WText *fineprint = addNew<WText>(tr("highscore.info"));
  fineprint->addStyleClass("fineprint");
}
