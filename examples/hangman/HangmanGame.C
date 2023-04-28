/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium
 *
 * See the LICENSE file for terms of use.
 */

#include "HangmanGame.h"

#include <Wt/WAnchor.h>
#include <Wt/WText.h>
#include <Wt/WStackedWidget.h>
#include <Wt/WApplication.h>
#include <Wt/Auth/AuthWidget.h>

#include "HangmanWidget.h"
#include "HighScoresWidget.h"

using namespace Wt;

HangmanGame::HangmanGame()
{
  session_.login().changed().connect(this, &HangmanGame::onAuthEvent);

  auto authModel = std::make_unique<Auth::AuthModel>(Session::auth(), session_.users());
  authModel->addPasswordAuth(&Session::passwordAuth());
  authModel->addOAuth(Session::oAuth());

  auto authWidget = std::make_unique<Auth::AuthWidget>(session_.login());
  auto authWidgetPtr = authWidget.get();
  authWidget->setModel(std::move(authModel));
  authWidget->setRegistrationEnabled(true);

  addNew<WText>("<h1>A Witty game: Hangman</h1>");

  addWidget(std::move(authWidget));

  mainStack_ = addNew<WStackedWidget>();
  mainStack_->setStyleClass("gamestack");

  links_ = addNew<WContainerWidget>();
  links_->setStyleClass("links");
  links_->hide();

  backToGameAnchor_ = links_->addNew<WAnchor>("/play", "Gaming Grounds");
  backToGameAnchor_->setLink(WLink(LinkType::InternalPath, "/play"));

  scoresAnchor_ = links_->addNew<WAnchor>("/highscores", "Highscores");
  scoresAnchor_->setLink(WLink(LinkType::InternalPath, "/highscores"));

  WApplication::instance()->internalPathChanged()
    .connect(this, &HangmanGame::handleInternalPath);

  authWidgetPtr->processEnvironment();
}

void HangmanGame::onAuthEvent()
{
  if (session_.login().loggedIn()) {
    links_->show();
    handleInternalPath(WApplication::instance()->internalPath());
  } else {
    mainStack_->clear();
    game_ = nullptr;
    scores_ = nullptr;
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
    scores_ = mainStack_->addNew<HighScoresWidget>(&session_);

  mainStack_->setCurrentWidget(scores_);
  scores_->update();

  backToGameAnchor_->removeStyleClass("selected-link");
  scoresAnchor_->addStyleClass("selected-link");
}

void HangmanGame::showGame()
{
  if (!game_) {
    game_ = mainStack_->addNew<HangmanWidget>(session_.userName());
    game_->scoreUpdated().connect(std::bind(&Session::addToScore, &session_, std::placeholders::_1));
  }

  mainStack_->setCurrentWidget(game_);

  backToGameAnchor_->addStyleClass("selected-link");
  scoresAnchor_->removeStyleClass("selected-link");
}
