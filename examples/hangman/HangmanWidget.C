/*
 * Copyright (C) 2011 Emweb bvba, Heverlee, Belgium
 *
 * See the LICENSE file for terms of use.
 */

#include "HangmanWidget.h"

#include <Wt/WText>
#include <Wt/WPushButton>
#include <Wt/WVBoxLayout>
#include <boost/lexical_cast.hpp>

#include "Session.h"
#include "WordWidget.h"
#include "ImagesWidget.h"
#include "LettersWidget.h"

using namespace Wt;

HangmanWidget::HangmanWidget(const std::string &name, 
			     Dictionary dictionary,
			     WContainerWidget *parent):
  WContainerWidget(parent),
  name_(name),
  dictionary_(dictionary)
{
  WVBoxLayout *layout = new WVBoxLayout();
  layout->setContentsMargins(0, 0, 0, 0);
  this->setLayout(layout);
  
  title_ = new WText("<h2>Guess the word!</h2>");
  layout->addWidget(title_, 0, AlignCenter | AlignMiddle);

  word_ = new WordWidget();
  layout->addWidget(word_, 0, AlignCenter | AlignMiddle);

  statusText_ = new WText();
  layout->addWidget(statusText_, 0, AlignCenter | AlignMiddle);

  images_ = new ImagesWidget();
  layout->addWidget(images_, 1, AlignCenter | AlignMiddle);

  letters_ = new LettersWidget();
  layout->addWidget(letters_, 0, AlignCenter | AlignMiddle);
  letters_->letterPushed().connect(this, &HangmanWidget::registerGuess);

  newGameButton_ = new WPushButton("New Game", this);
  newGameButton_->clicked().connect(this, &HangmanWidget::newGame);
  layout->addWidget(newGameButton_, 0, AlignCenter | AlignMiddle);

  // prepare for first game
  newGame();
}

void HangmanWidget::newGame()
{
  WString title("<h2>Guess the word, {1}!</h2>");
  title_->setText(title.arg(name_));

  newGameButton_->hide(); // don't let the player chicken out

  // Bring widget to initial state
  word_->init(RandomWord(dictionary_));
  images_->init();
  letters_->init();

  statusText_->setText("");
}

void HangmanWidget::registerGuess(char c)
{
  bool correct = word_->guess(c);

  if (!correct)
    images_->badGuess();

  if (images_->gameOver()) {
    WString status("You hang... <br />The correct answer was: {1} ");
    statusText_->setText(status.arg(word_->word()));

    letters_->hide();
    newGameButton_->show();

    updateScore_.emit(-10);
  } else if (word_->won()) {
    statusText_->setText("You win!");
    images_->hurray();

    letters_->hide();
    newGameButton_->show();

    updateScore_.emit(20 - images_->badGuesses());
  }
}
