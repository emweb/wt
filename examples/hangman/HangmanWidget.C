/*
 * Copyright (C) 2011 Emweb bvba, Heverlee, Belgium
 *
 * See the LICENSE file for terms of use.
 */

#include "HangmanWidget.h"

#include <Wt/WTable>
#include <Wt/WText>
#include <Wt/WLineEdit>
#include <Wt/WPushButton>
#include <Wt/WImage>
#include <Wt/WSignalMapper>
#include <Wt/WVBoxLayout>
#include <boost/lexical_cast.hpp>

#include "HangmanApplication.h"

using namespace Wt;

HangmanWidget::HangmanWidget(WContainerWidget *parent):
  WContainerWidget(parent),
  maxGuesses_(9)
{
  WVBoxLayout *layout = new WVBoxLayout();
  layout->setContentsMargins(0, 0, 0, 0);
  this->setLayout(layout);
  
  title_ = new WText("<h2>Guess the word!</h2>");
  layout->addWidget(title_, 0, AlignCenter | AlignMiddle);

  wordContainer_ = new WContainerWidget();
  wordContainer_->addStyleClass("wordcontainer");
  layout->addWidget(wordContainer_, 0, AlignCenter | AlignMiddle);

  statusText_ = new WText(this);
  layout->addWidget(statusText_, 0, AlignCenter | AlignMiddle);

  WContainerWidget *images = createHangmanImages();
  layout->addWidget(images, 1, AlignCenter | AlignMiddle);

  letterButtonLayout_ = createAlphabetTable();
  layout->addWidget(letterButtonLayout_, 0, AlignCenter | AlignMiddle);

  newGameButton_ = new WPushButton("New Game", this);
  newGameButton_->clicked().connect(this, &HangmanWidget::newGame);
  layout->addWidget(newGameButton_, 0, AlignCenter | AlignMiddle);

  // prepare for first game
  newGame();
}

WContainerWidget *HangmanWidget::createHangmanImages()
{
  WContainerWidget *parent = new WContainerWidget();

  for(unsigned int i = 0; i <= maxGuesses_; ++i) {
    std::string fname = "icons/hangman";
    fname += boost::lexical_cast<std::string>(i) + ".png";
    WImage *theImage = new WImage(fname, parent);
    hangmanImages_.push_back(theImage);
    
    // Although not necessary, we can avoid flicker (on konqueror)
    // by presetting the image size.
    theImage->resize(256, 256);
  }

  hurrayImage_ = new WImage("icons/hangmanhurray.png", parent);
  resetImages(); // Hide all images

  return parent;
}

WTable *HangmanWidget::createAlphabetTable()
{
  WTable *letterButtonLayout = new WTable();

  // The default width of a table is 100%...
  letterButtonLayout->resize(13*30, WLength::Auto);

  WSignalMapper<WPushButton *> *mapper
    = new WSignalMapper<WPushButton *>(this);

  for(unsigned int i = 0; i < 26; ++i) {
    std::wstring c(1, 'A' + i);
    WPushButton *character =
      new WPushButton(c, letterButtonLayout->elementAt(i / 13, i % 13));
    letterButtons_.push_back(character);
    character->resize(30, WLength::Auto);
    mapper->mapConnect(character->clicked(), character);
  }

  mapper->mapped().connect(this, &HangmanWidget::processButton);

  return letterButtonLayout;
}

void HangmanWidget::newGame()
{
  word_ = RandomWord(HangmanApplication::instance()->dictionary);
  title_->setText("<h2>Guess the word, " 
		 + HangmanApplication::instance()->user->name + "!</h2>");
  newGameButton_->hide(); // don't let the player chicken out

  // Bring widget to initial state
  resetImages();
  resetButtons();
  badGuesses_ = displayedLetters_ = 0;
  hangmanImages_[0]->show();

  // Prepare the widgets for the new word
  wordContainer_->clear();
  wordLetters_.clear();
  for(unsigned int i = 0; i < word_.size(); ++i) {
    WText *c = new WText("-", wordContainer_);
    wordLetters_.push_back(c);
  }

  // resize appropriately so that the border nooks nice.
  wordContainer_->resize(WLength(word_.size() * 1.5, WLength::FontEx),
			WLength::Auto);

  statusText_->setText("");
}

void HangmanWidget::processButton(WPushButton *button)
{
  if (!button->isEnabled())
    return;

  wchar_t c = button->text().value().c_str()[0];
  if(std::find(word_.begin(), word_.end(), c) != word_.end())
    registerCorrectGuess(c);
  else
    registerBadGuess();
  button->disable();
}

void HangmanWidget::registerBadGuess()
{
  if(badGuesses_ < maxGuesses_) {
    hangmanImages_[badGuesses_]->hide();
    badGuesses_++;
    hangmanImages_[badGuesses_]->show();
    if(badGuesses_ == maxGuesses_) {
      statusText_->setText(L"You hang... <br />"
			  L"The correct answer was: " + word_);
      letterButtonLayout_->hide();
      newGameButton_->show();
      HangmanApplication::instance()->user.modify()->score -= 10;
    }
  }
}

void HangmanWidget::registerCorrectGuess(wchar_t c)
{
  for(unsigned int i = 0; i < word_.size(); ++i) {
    if(word_[i] == c) {
      displayedLetters_++;
      wordLetters_[i]->setText(std::wstring(1, c));
    }
  }
  if(displayedLetters_ == word_.size()) {
    statusText_->setText("You win!");
    hangmanImages_[badGuesses_]->hide();
    hurrayImage_->show();
    letterButtonLayout_->hide();
    newGameButton_->show();
    HangmanApplication::instance()->user.modify()->score += (20 - badGuesses_);
  }
}

void HangmanWidget::resetImages()
{
  hurrayImage_->hide();
  for(unsigned int i = 0; i < hangmanImages_.size(); ++i)
    hangmanImages_[i]->hide();
}

void HangmanWidget::resetButtons()
{
  for(unsigned int i = 0; i < letterButtons_.size(); ++i) {
    letterButtons_[i]->enable();
  }
  letterButtonLayout_->show();
}
