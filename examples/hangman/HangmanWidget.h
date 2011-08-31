// This may look like C code, but it's really -*- C++ -*-
/* 
 * Copyright (C) 2011 Emweb bvba, Heverlee, Belgium
 *
 * See the LICENSE file for terms of use.
 */

#ifndef HANGMANWIDGET_H_
#define HANGMANWIDGET_H_

#include <vector>

#include <Wt/WContainerWidget>

namespace Wt {
  class WText;
  class WPushButton;
  class WImage;
  class WLineEdit;
  class WPushButton;
  class WTable;
}

class HangmanWidget: public Wt::WContainerWidget
{
public:
  HangmanWidget(Wt::WContainerWidget *parent = 0);
  
private:
  Wt::WText                     *title_;
  Wt::WTable                    *letterButtonLayout_;
  std::vector<Wt::WPushButton *> letterButtons_;
  std::vector<Wt::WImage *>      hangmanImages_;
  Wt::WImage                    *hurrayImage_;
  Wt::WContainerWidget          *wordContainer_;
  Wt::WText                     *statusText_;
  std::vector<Wt::WText *>       wordLetters_;
  Wt::WPushButton               *newGameButton_;
      
  const unsigned int maxGuesses_;
  unsigned int       badGuesses_;
  unsigned int       displayedLetters_;
  std::wstring       word_;
  
  // constructor helpers
  Wt::WTable *createAlphabetTable();
  Wt::WContainerWidget *createHangmanImages();
  
  // other helpers
  void resetImages();
  void resetButtons();
  void registerBadGuess();
  void registerCorrectGuess(wchar_t c);
  void processButton(Wt::WPushButton *button);
  void newGame();
};

#endif //HANGMANWIDGET_H_
