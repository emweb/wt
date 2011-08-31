/*
 * Copyright (C) 2005 Wim Dumon
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WAnchor>
#include <Wt/WText>
#include <Wt/WStackedWidget>
#include <Wt/WVBoxLayout>
#include <Wt/WHBoxLayout>

#include "HangmanGame.h"
#include "LoginWidget.h"
#include "HangmanWidget.h"
#include "HighScoresWidget.h"
#include "HangmanApplication.h"

using namespace Wt;

HangmanGame::HangmanGame(WContainerWidget *parent):
  WContainerWidget(parent),
  game_(0),
  scores_(0)
{
   WVBoxLayout *layout = new WVBoxLayout();
   layout->setContentsMargins(0, 0, 0, 0);
   this->setLayout(layout);

   WText *title = new WText("<h1>A Witty game: Hangman</h1>");
   layout->addWidget(title);

   mainStack_ = new WStackedWidget(this);
   mainStack_->setPadding(20);
   layout->addWidget(mainStack_, 1, AlignCenter | AlignMiddle);
   
   mainStack_->addWidget(login_ = new LoginWidget());

   WHBoxLayout *linksLayout = new WHBoxLayout();
   linksLayout->setContentsMargins(0, 0, 0, 0);
   layout->addLayout(linksLayout, 0, AlignCenter | AlignMiddle);

   backToGameAnchor_ = new WAnchor("/play", "Gaming Grounds");
   linksLayout->addWidget(backToGameAnchor_, 0, AlignCenter | AlignMiddle);
   backToGameAnchor_->setRefInternalPath("/play");
   backToGameAnchor_->addStyleClass("link");

   scoresAnchor_ = new WAnchor("/highscores", "Highscores");
   linksLayout->addWidget(scoresAnchor_, 0, AlignCenter | AlignMiddle);
   scoresAnchor_->setRefInternalPath("/highscores");
   scoresAnchor_->addStyleClass("link");

   HangmanApplication::instance()
     ->internalPathChanged().connect(this, &HangmanGame::handleInternalPath);

   handleInternalPath();
}

void HangmanGame::handleInternalPath()
{
  HangmanApplication *app = HangmanApplication::instance();

  if (app->internalPath() == "/play" && app->user)
    showGame();
  else if (app->internalPath() == "/highscores")
    showHighScores();
  else
    showLogin();
}

void HangmanGame::showLogin()
{
  HangmanApplication::instance()->setInternalPath("/");

  mainStack_->setCurrentWidget(login_);
  backToGameAnchor_->hide();
  scoresAnchor_->hide();
}

void HangmanGame::showHighScores()
{
  if (!scores_)
    scores_ = new HighScoresWidget(mainStack_);

  mainStack_->setCurrentWidget(scores_);
  scores_->update();

  backToGameAnchor_->show();
  scoresAnchor_->show();
  
  backToGameAnchor_->removeStyleClass("selected-link");
  scoresAnchor_->addStyleClass("selected-link");
}

void HangmanGame::showGame()
{
  if (!game_)
    game_ = new HangmanWidget(mainStack_);

  mainStack_->setCurrentWidget(game_);

  backToGameAnchor_->show();
  scoresAnchor_->show();
    
  backToGameAnchor_->addStyleClass("selected-link");
  scoresAnchor_->removeStyleClass("selected-link");
}

