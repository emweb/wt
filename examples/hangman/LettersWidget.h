// This may look like C code, but it's really -*- C++ -*-
/* 
 * Copyright (C) 2011 Emweb bvba, Heverlee, Belgium
 *
 * See the LICENSE file for terms of use.
 */

#ifndef LETTERS_WIDGET_H_
#define LETTERS_WIDGET_H_

#include <Wt/WCompositeWidget>

namespace Wt {
  class WContainerWidget;
  class WPushButton;
  class WTable;
}

class LettersWidget : public Wt::WCompositeWidget
{
public:
  LettersWidget(Wt::WContainerWidget *parent = 0);

  void reset();

  Wt::Signal<char>& letterPushed() { return letterPushed_; } 

private:
  Wt::WTable                     *impl_;
  std::vector<Wt::WPushButton *>  letterButtons_;

  Wt::Signal<char>                letterPushed_;

  void processButton(Wt::WPushButton *b);
  void processButtonPushed(Wt::WKeyEvent &e, Wt::WPushButton *b);
};

#endif //LETTERS_WIDGET_H_
