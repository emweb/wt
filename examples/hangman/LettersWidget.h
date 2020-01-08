// This may look like C code, but it's really -*- C++ -*-
/* 
 * Copyright (C) 2011 Emweb bv, Herent, Belgium
 *
 * See the LICENSE file for terms of use.
 */

#ifndef LETTERS_WIDGET_H_
#define LETTERS_WIDGET_H_

#include <Wt/WCompositeWidget.h>

using namespace Wt;

namespace Wt {
  class WContainerWidget;
  class WPushButton;
  class WTable;
}

class LettersWidget : public WCompositeWidget
{
public:
  LettersWidget();
  virtual ~LettersWidget();

  void reset();

  Signal<char>& letterPushed() { return letterPushed_; }

private:
  WTable                     *impl_;
  std::vector<WPushButton *>  letterButtons_;
  std::vector<Wt::Signals::connection> connections_;

  Signal<char>                letterPushed_;

  void processButton(WPushButton *b);
  void processButtonPushed(const WKeyEvent &e, WPushButton *b);
};

#endif //LETTERS_WIDGET_H_
