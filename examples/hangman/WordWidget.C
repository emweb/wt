/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium
 *
 * See the LICENSE file for terms of use.
 */

#include "WordWidget.h"

#include <Wt/WText.h>

WordWidget::WordWidget()
{
  addStyleClass("wordcontainer");
}

void WordWidget::init(const std::string &word)
{
  word_ = word;
  displayedLetters_ = 0;

  clear();
  wordLetters_.clear();
  for (const char ch : word_) {
    auto c = addNew<Wt::WText>("-");
    wordLetters_.push_back(c);
  }
}

bool WordWidget::guess(char c)
{
  bool correct = false;

  for (std::size_t i = 0; i < word_.size(); ++i) {
    if(word_[i] == c) {
      displayedLetters_++;
      wordLetters_[i]->setText(std::string(1, c));
      correct = true;
    }
  }

  return correct;
}

bool WordWidget::won()
{
  return displayedLetters_ == word_.size();
}
