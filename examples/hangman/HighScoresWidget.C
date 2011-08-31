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
#include "HangmanApplication.h"

using namespace Wt;

HighScoresWidget::HighScoresWidget(WContainerWidget *parent):
  WContainerWidget(parent)
{
  setContentAlignment(AlignCenter);
  setStyleClass("highscores");
}

void HighScoresWidget::update()
{
  clear();
  
  HangmanApplication *app = HangmanApplication::instance();
  
  WText *title = new WText("<h2>Hall of fame</h2>", this);
  
  int ranking = app->user->findRanking(app->session);
  
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
  
  Dbo::Transaction transaction(app->session);
  Users top = app->session.find<User>().orderBy("score desc").limit(20);

  WTable *table = new WTable(this);

  new WText("Rank", table->elementAt(0, 0));
  new WText("User", table->elementAt(0, 1));
  new WText("Games", table->elementAt(0, 2));
  new WText("Score", table->elementAt(0, 3));
  new WText("Last game", table->elementAt(0, 4));
  table->setHeaderCount(1);

  int formerScore = -1;
  int rank = 0;
  for (Users::const_iterator i = top.begin(); i != top.end(); ++i) {
    if ((*i)->score != formerScore) {
      formerScore = (*i)->score;
      ++rank;
    }
    
    int row = table->rowCount();
    new WText(boost::lexical_cast<std::string>(rank),
	      table->elementAt(row, 0));
    new WText((*i)->name, table->elementAt(row, 1));
    new WText(boost::lexical_cast<std::string>((*i)->gamesPlayed),
	      table->elementAt(row, 2));
    new WText(boost::lexical_cast<std::string>((*i)->score),
	      table->elementAt(row, 3));
    new WText((*i)->lastLogin.toString(), table->elementAt(row, 4));
    
    if (app->user && (*i) == app->user)
      table->rowAt(row)->setId("self");
  }

  WText *fineprint = new WText(tr("highscore.info"), this);
  fineprint->addStyleClass("fineprint");
}
