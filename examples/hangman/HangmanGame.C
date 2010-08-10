/*
 * Copyright (C) 2005 Wim Dumon
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WText>
#include <Wt/WTable>
#include <Wt/WTableCell>
#include <Wt/WStackedWidget>
#include <Wt/WCssDecorationStyle>

#include "HangmanGame.h"
#include "LoginWidget.h"
#include "HangmanWidget.h"
#include "HighScoresWidget.h"

HangmanGame::HangmanGame(WContainerWidget *parent):
   WTable(parent)
{
   resize(WLength(100, WLength::Percentage), WLength::Auto);

   WText *title = new WText(L"A Witty game: Hangman", elementAt(0,0));
   title->decorationStyle().font().setSize(WFont::XXLarge);

   // Center the title horizontally.
   elementAt(0, 0)->setContentAlignment(AlignTop | AlignCenter);

   // Element (1,1) holds a stack of widgets with the main content.
   // This is where we switch between Login, Game, and Highscores widgets.
   MainStack = new WStackedWidget(elementAt(1, 0));
   MainStack->setPadding(20);

   MainStack->addWidget(Login = new LoginWidget());
   Login->loginSuccessful.connect(this, &HangmanGame::play);

   // Element (2,0) contains navigation buttons. Instead of WButton,
   // we use WText. WText inherits from WInteractWidget, and thus exposes
   // the click event.
   BackToGameText = new WText(L" Gaming Grounds ", elementAt(2, 0));
   BackToGameText->decorationStyle().setCursor(PointingHandCursor);
   BackToGameText->clicked().connect(this, &HangmanGame::showGame);

   ScoresText = new WText(L" Highscores ", elementAt(2, 0));
   ScoresText->decorationStyle().setCursor(PointingHandCursor);
   ScoresText->clicked().connect(this, &HangmanGame::showHighScores);
   // Center the buttons horizontally.
   elementAt(2, 0)->setContentAlignment(AlignTop | AlignCenter);

   doLogin();
}

void HangmanGame::doLogin()
{
   MainStack->setCurrentWidget(Login);
   BackToGameText->hide();
   ScoresText->hide();
}

void HangmanGame::play(std::wstring user, Dictionary dict)
{
   // Add a widget by passing MainStack as the parent, ...
   Game = new HangmanWidget(user, dict, MainStack);
   // ... or using addWidget
   MainStack->addWidget(Scores = new HighScoresWidget(user));

   BackToGameText->show();
   ScoresText->show();

   showGame();
}

void HangmanGame::showHighScores()
{
   MainStack->setCurrentWidget(Scores);
   Scores->update();
   BackToGameText->decorationStyle().font().setWeight(WFont::NormalWeight);
   ScoresText->decorationStyle().font().setWeight(WFont::Bold);
}

void HangmanGame::showGame()
{
   MainStack->setCurrentWidget(Game);
   BackToGameText->decorationStyle().font().setWeight(WFont::Bold);
   ScoresText->decorationStyle().font().setWeight(WFont::NormalWeight);
}
