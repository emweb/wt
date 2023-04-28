// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium
 *
 * See the LICENSE file for terms of use.
 */

#ifndef HANGMAN_WIDGET_H_
#define HANGMAN_WIDGET_H_

#include <Wt/WContainerWidget.h>

#include <vector>

class WordWidget;
class ImagesWidget;
class LettersWidget;

class HangmanWidget : public Wt::WContainerWidget
{
public:
  explicit HangmanWidget(const std::string &name);

  Wt::Signal<int>& scoreUpdated() { return scoreUpdated_; }

private:
  Wt::WText *title_ = nullptr;

  WordWidget *word_ = nullptr;
  ImagesWidget *images_ = nullptr;
  LettersWidget *letters_ = nullptr;

  Wt::WText *statusText_ = nullptr;
  Wt::WComboBox *language_ = nullptr;
  Wt::WPushButton *newGameButton_ = nullptr;

  Wt::Signal<int> scoreUpdated_;

  std::string name_;

  int badGuesses_ = 0;

  void registerGuess(char c);

  void newGame();
};

#endif //HANGMAN_WIDGET_H_
