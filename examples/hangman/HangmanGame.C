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
#include <Wt/Auth/AuthWidget>

#include "HangmanGame.h"
#include "HangmanWidget.h"
#include "HighScoresWidget.h"

using namespace Wt;

HangmanGame::HangmanGame(WContainerWidget *parent):
  WContainerWidget(parent),
  game_(0),
  scores_(0)
{
  session_.login().changed().connect(this, &HangmanGame::onAuthEvent);

  Auth::AuthModel *authModel = new Auth::AuthModel(Session::auth(),
						   session_.users(), this);
  authModel->addPasswordAuth(&Session::passwordAuth());
  authModel->addOAuth(Session::oAuth());

  Auth::AuthWidget *authWidget = new Auth::AuthWidget(session_.login());
  authWidget->setModel(authModel);
  authWidget->setRegistrationEnabled(true);

  WText *title = new WText("<h1>A Witty game: Hangman</h1>");
  addWidget(title);

  addWidget(authWidget);

  mainStack_ = new WStackedWidget();
  mainStack_->setStyleClass("gamestack");
  addWidget(mainStack_);

  links_ = new WContainerWidget();
  links_->setStyleClass("links");
  links_->hide();
  addWidget(links_);

  backToGameAnchor_ = new WAnchor("/play", "Gaming Grounds", links_);
  backToGameAnchor_->setLink(WLink(WLink::InternalPath, "/play"));

  scoresAnchor_ = new WAnchor("/highscores", "Highscores", links_);
  scoresAnchor_->setLink(WLink(WLink::InternalPath, "/highscores"));

  WApplication::instance()->internalPathChanged()
    .connect(this, &HangmanGame::handleInternalPath);

  authWidget->processEnvironment();
}

void HangmanGame::onAuthEvent()
{
  if (session_.login().loggedIn()) {  
    links_->show();
    handleInternalPath(WApplication::instance()->internalPath());
  } else {
    mainStack_->clear();
    game_ = 0;
    scores_ = 0;
    links_->hide();
  }
}

void HangmanGame::handleInternalPath(const std::string &internalPath)
{
  if (session_.login().loggedIn()) {
    if (internalPath == "/play")
      showGame();
    else if (internalPath == "/highscores")
      showHighScores();
    else
      WApplication::instance()->setInternalPath("/play",  true);
  }
}

void HangmanGame::showHighScores()
{
  if (!scores_)
    scores_ = new HighScoresWidget(&session_, mainStack_);

  mainStack_->setCurrentWidget(scores_);
  scores_->update();

  backToGameAnchor_->removeStyleClass("selected-link");
  scoresAnchor_->addStyleClass("selected-link");
}

void HangmanGame::showGame()
{
  if (!game_) {
    game_ = new HangmanWidget(session_.userName(), mainStack_);
    game_->scoreUpdated().connect(&session_, &Session::addToScore);
  }

  mainStack_->setCurrentWidget(game_);

  backToGameAnchor_->addStyleClass("selected-link");
  scoresAnchor_->removeStyleClass("selected-link");
}
