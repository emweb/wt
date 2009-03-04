/*
 * Copyright (C) 2005 Wim Dumon
 *
 * See the LICENSE file for terms of use.
 */

#include <boost/lexical_cast.hpp>
#include <sstream>

#include <Wt/WBreak>
#include <Wt/WText>
#include <Wt/WTable>
#include <Wt/WTableCell>
#include <Wt/WCssDecorationStyle>

#include "HighScoresWidget.h"
#include "HangmanDb.h"

using namespace std;

HighScoresWidget::HighScoresWidget(const std::wstring &user,
				   WContainerWidget *parent):
   WContainerWidget(parent),
   User(user)
{
   setContentAlignment(AlignCenter);
   setStyleClass("highscores");
}

void HighScoresWidget::update()
{
   clear();

   WText *title = new WText("Hall of fame", this);
   title->decorationStyle().font().setSize(WFont::XLarge);
   title->setMargin(10, Top | Bottom);

   new WBreak(this);

   HangmanDb::Score s = HangmanDb::getUserPosition(User);

   std::string yourScore;
   if (s.number == 1)
     yourScore = "Congratulations! You are currently leading the pack.";
   else {
     yourScore = "You are currently ranked number "
       + boost::lexical_cast<std::string>(s.number)
       + ". Almost there !";
   }

   WText *score = new WText("<p>" + yourScore + "</p>", this);
   score->decorationStyle().font().setSize(WFont::Large);

   std::vector<HangmanDb::Score> top = HangmanDb::getHighScores(20);

   WTable *table = new WTable(this);
   new WText("Rank", table->elementAt(0, 0));
   new WText("User", table->elementAt(0, 1));
   new WText("Games", table->elementAt(0, 2));
   new WText("Score", table->elementAt(0, 3));
   new WText("Last game", table->elementAt(0, 4));
   for(unsigned int i = 0; i < top.size(); ++i) {
      new WText(boost::lexical_cast<string>(top[i].number),
		table->elementAt(i + 1, 0));
      new WText(top[i].user, table->elementAt(i + 1, 1));
      new WText(boost::lexical_cast<std::string>(top[i].numgames),
		table->elementAt(i+ 1, 2));
      new WText(boost::lexical_cast<std::string>(top[i].score),
		table->elementAt(i + 1, 3));
      new WText(top[i].lastseen, table->elementAt(i + 1, 4));
   }

   table->resize(WLength(60, WLength::FontEx), WLength::Auto);
   table->setMargin(20, Top | Bottom);
   table->decorationStyle().setBorder(WBorder(WBorder::Solid));

   /*
    * Apply cell styles
    */
   for (int row = 0; row < table->rowCount(); ++row) {
     for (int col = 0; col < table->columnCount(); ++col) {
       WTableCell *cell = table->elementAt(row, col);
       cell->setContentAlignment(AlignMiddle | AlignCenter);

       if (row == 0)
	 cell->setStyleClass("highscoresheader");

       if (row == s.number)
	 cell->setStyleClass("highscoresself");
     }
   }

   WText *fineprint
     = new WText("<p>For each game won, you gain 20 points, "
		 "minus one point for each wrong letter guess.<br />"
		 "For each game lost, you loose 10 points, so you "
		 "better try hard to guess the word!</p>", this);
   fineprint->decorationStyle().font().setSize(WFont::Smaller);
}
