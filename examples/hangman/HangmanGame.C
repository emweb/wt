/*
 * Copyright (C) 2011 Emweb bvba, Heverlee, Belgium
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WAnchor>
#include <Wt/WText>
#include <Wt/WStackedWidget>
#include <Wt/WVBoxLayout>
#include <Wt/WHBoxLayout>
#include <Wt/WApplication>

#include "HangmanGame.h"
#include "LoginWidget.h"
#include "HangmanWidget.h"
#include "HighScoresWidget.h"

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
  
  mainStack_->addWidget(login_ = new LoginWidget(&session_));
  login_->loggedIn().connect(this, &HangmanGame::onLogin);

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

  WApplication::instance()->internalPathChanged()
    .connect(this, &HangmanGame::handleInternalPath);

  showLogin();
}

void HangmanGame::handleInternalPath(const std::string &internalPath)
{
  if (internalPath == "/play" && session_.user())
    showGame();
  else if (internalPath == "/highscores")
    showHighScores();
  else
    showLogin();
}

void HangmanGame::showLogin()
{
  mainStack_->setCurrentWidget(login_);
  backToGameAnchor_->hide();
  scoresAnchor_->hide();
}

void HangmanGame::showHighScores()
{
  if (!scores_)
    scores_ = new HighScoresWidget(&session_, mainStack_);

  mainStack_->setCurrentWidget(scores_);
  scores_->update();

  backToGameAnchor_->show();
  scoresAnchor_->show();
  
  backToGameAnchor_->removeStyleClass("selected-link");
  scoresAnchor_->addStyleClass("selected-link");
}

void HangmanGame::onLogin()
{
  WApplication *app = WApplication::instance();

  std::string path = app->internalPath();
  if (path != "/highscores" && path != "/play")
    app->setInternalPath("/play", true);
  else
    handleInternalPath(path);
}

void HangmanGame::showGame()
{
  if (!game_) {
    game_ = new HangmanWidget(session_.user()->name, 
			      session_.dictionary(),
			      mainStack_);
    game_->updateScore().connect(&session_, &Session::addToScore);
  }

  mainStack_->setCurrentWidget(game_);

  backToGameAnchor_->show();
  scoresAnchor_->show();
    
  backToGameAnchor_->addStyleClass("selected-link");
  scoresAnchor_->removeStyleClass("selected-link");
}
