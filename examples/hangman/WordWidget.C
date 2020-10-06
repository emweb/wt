/* 
 * Copyright (C) 2011 Emweb bv, Herent, Belgium
 *
 * See the LICENSE file for terms of use.
 */

#include "WordWidget.h"

#include <Wt/WText.h>

using namespace Wt;

WordWidget::WordWidget() :
  WContainerWidget()
{
  addStyleClass("wordcontainer");
}

void WordWidget::init(const std::wstring &word)
{
  word_ = word;
  displayedLetters_ = 0;

  clear();
  wordLetters_.clear();
  for(unsigned int i = 0; i < word_.size(); ++i) {
    WText *c = this->addWidget(std::make_unique<WText>("-"));
    wordLetters_.push_back(c);
  }
}

bool WordWidget::guess(wchar_t c)
{
  bool correct = false;

  for(unsigned int i = 0; i < word_.size(); ++i) {
    if(word_[i] == c) {
      displayedLetters_++;
      wordLetters_[i]->setText(std::wstring(1, c));
      correct = true;
    }
  }

  return correct;
}

bool WordWidget::won()
{
  return displayedLetters_ == word_.size();
}
