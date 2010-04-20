/* this is a -*-C++-*- file
 *
 * Copyright (C) 2005 Wim Dumon
 *
 * See the LICENSE file for terms of use.
 */

#ifndef HANGMANWIDGET_H_
#define HANGMANWIDGET_H_

#include <Wt/WContainerWidget>
#include <Wt/WText>
#include <Wt/WPushButton>
#include <vector>

#include "Dictionary.h"

namespace Wt {
  class WPushButton;
  class WImage;
  class WLineEdit;
  class WPushButton;
  class WTable;
}

using namespace Wt;

class HangmanWidget: public WContainerWidget
{
   public:
      HangmanWidget(std::wstring user, Dictionary dict,
		    WContainerWidget *parent = 0);

   private:
      WText                     *Title;
      WTable                    *LetterButtonLayout;
      std::vector<WPushButton *> LetterButtons;
      std::vector<WImage *>      HangmanImages;
      WImage                    *HurrayImage;
      WContainerWidget          *WordContainer;
      WText                     *StatusText;
      std::vector<WText *>       WordLetters;
      WPushButton               *NewGameButton;
      
      const unsigned int MaxGuesses;
      unsigned int       BadGuesses;
      unsigned int       DisplayedLetters;
      std::wstring       Word;
      std::wstring       User;
      Dictionary         Dict;

      // constructor helpers
      void createAlphabet(WContainerWidget *parent);
      void createHangmanImages(WContainerWidget *parent);

      // other helpers
      void resetImages();
      void resetButtons();
      void registerBadGuess();
      void registerCorrectGuess(wchar_t c);
      void processButton(WPushButton *button);
      void newGame();
};

#endif
