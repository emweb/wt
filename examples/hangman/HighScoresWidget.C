/*
 * Copyright (C) 2011 Emweb bvba, Heverlee, Belgium
 *
 * See the LICENSE file for terms of use.
 */

#include <boost/lexical_cast.hpp>

#include <Wt/WText>
#include <Wt/WTable>
#include <Wt/Dbo/Dbo>

#include "HighScoresWidget.h"
#include "Session.h"

using namespace Wt;

HighScoresWidget::HighScoresWidget(Session *session, WContainerWidget *parent):
  WContainerWidget(parent),
  session_(session)
{
  setContentAlignment(AlignCenter);
  setStyleClass("highscores");
}

void HighScoresWidget::update()
{
  clear();
  
  new WText("<h2>Hall of fame</h2>", this);
  
  int ranking = session_->findRanking();
  
  std::string yourScore;
  if (ranking == 1)
    yourScore = "Congratulations! You are currently leading the pack.";
  else {
    yourScore = "You are currently ranked number "
      + boost::lexical_cast<std::string>(ranking)
      + ". Almost there !";
  }

  WText *score = new WText("<p>" + yourScore + "</p>", this);
  score->addStyleClass("score");
  
  std::vector<User> top = session_->topUsers(20);

  WTable *table = new WTable(this);

  new WText("Rank", table->elementAt(0, 0));
  new WText("User", table->elementAt(0, 1));
  new WText("Games", table->elementAt(0, 2));
  new WText("Score", table->elementAt(0, 3));
  new WText("Last game", table->elementAt(0, 4));
  table->setHeaderCount(1);

  int formerScore = -1;
  int rank = 0;
  for (unsigned i = 0; i < top.size(); i++) {
    User u = top[i];

    if (u.score != formerScore) {
      formerScore = u.score;
      ++rank;
    }
    
    int row = table->rowCount();
    new WText(boost::lexical_cast<std::string>(rank),
	      table->elementAt(row, 0));
    new WText(u.name, table->elementAt(row, 1));
    new WText(boost::lexical_cast<std::string>(u.gamesPlayed),
	      table->elementAt(row, 2));
    new WText(boost::lexical_cast<std::string>(u.score),
	      table->elementAt(row, 3));
    if (!u.lastGame.isNull())
      new WText(u.lastGame.timeTo(WDateTime::currentDateTime())
		+ " ago", table->elementAt(row, 4));
    else
      new WText("---", table->elementAt(row, 4));
    
    if (session_->login().loggedIn() && session_->userName() == u.name)
      table->rowAt(row)->setId("self");
  }

  WText *fineprint = new WText(tr("highscore.info"), this);
  fineprint->addStyleClass("fineprint");
}
