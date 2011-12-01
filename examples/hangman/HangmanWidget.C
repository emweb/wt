/*
 * Copyright (C) 2011 Emweb bvba, Heverlee, Belgium
 *
 * See the LICENSE file for terms of use.
 */

#include "HangmanWidget.h"

#include <Wt/WComboBox>
#include <Wt/WPushButton>
#include <Wt/WText>
#include <Wt/WVBoxLayout>
#include <boost/lexical_cast.hpp>

#include "Session.h"
#include "WordWidget.h"
#include "ImagesWidget.h"
#include "LettersWidget.h"

using namespace Wt;

namespace {
  const int MaxGuesses = 9;
}

HangmanWidget::HangmanWidget(const std::string &name, WContainerWidget *parent)
  : WContainerWidget(parent),
    name_(name),
    badGuesses_(0)
{
  WVBoxLayout *layout = new WVBoxLayout();
  layout->setContentsMargins(0, 0, 0, 0);
  this->setLayout(layout);
  
  title_ = new WText(tr("hangman.readyToPlay"));
  layout->addWidget(title_, 0, AlignCenter | AlignMiddle);

  word_ = new WordWidget();
  layout->addWidget(word_, 0, AlignCenter | AlignMiddle);

  statusText_ = new WText();
  layout->addWidget(statusText_, 0, AlignCenter | AlignMiddle);

  images_ = new ImagesWidget(MaxGuesses);
  layout->addWidget(images_, 1, AlignCenter | AlignMiddle);

  letters_ = new LettersWidget();
  layout->addWidget(letters_, 0, AlignCenter | AlignMiddle);
  letters_->letterPushed().connect(this, &HangmanWidget::registerGuess);

  language_ = new WComboBox();
  language_->addItem(tr("hangman.englishWords").arg(18957));
  language_->addItem(tr("hangman.dutchWords").arg(1688));
  layout->addWidget(language_, 0, AlignCenter | AlignMiddle);

  newGameButton_ = new WPushButton(tr("hangman.newGame"), this);
  newGameButton_->clicked().connect(this, &HangmanWidget::newGame);
  layout->addWidget(newGameButton_, 0, AlignCenter | AlignMiddle);

  letters_->hide();
}

void HangmanWidget::newGame()
{
  WString title(tr("hangman.guessTheWord"));
  title_->setText(title.arg(name_));

  language_->hide();
  newGameButton_->hide();

  /*
   * Choose a new secret word and reset the game
   */
  Dictionary dictionary = (Dictionary) language_->currentIndex();
  word_->init(RandomWord(dictionary));
  letters_->reset();
  badGuesses_ = 0;

  images_->showImage(badGuesses_);
  statusText_->setText("");
}

void HangmanWidget::registerGuess(char c)
{
  bool correct = word_->guess(c);

  if (!correct) {
    ++badGuesses_;
    images_->showImage(badGuesses_);
  }

  if (badGuesses_ == MaxGuesses) {
    WString status(tr("hangman.youHang"));
    statusText_->setText(status.arg(word_->word()));

    letters_->hide();
    language_->show();
    newGameButton_->show();

    scoreUpdated_.emit(-10);
  } else if (word_->won()) {
    statusText_->setText(tr("hangman.youWin"));
    images_->showImage(ImagesWidget::HURRAY);

    letters_->hide();
    language_->show();
    newGameButton_->show();

    scoreUpdated_.emit(20 - badGuesses_);
  }
}
