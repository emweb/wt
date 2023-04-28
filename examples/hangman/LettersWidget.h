// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium
 *
 * See the LICENSE file for terms of use.
 */

#ifndef LETTERS_WIDGET_H_
#define LETTERS_WIDGET_H_

#include <Wt/WCompositeWidget.h>

namespace Wt {
  class WContainerWidget;
  class WPushButton;
  class WTable;
}

class LettersWidget : public Wt::WCompositeWidget
{
public:
  LettersWidget();
  virtual ~LettersWidget();

  void reset();

  Wt::Signal<char>& letterPushed() { return letterPushed_; }

private:
  Wt::WTable *impl_ = nullptr;
  std::vector<Wt::WPushButton*> letterButtons_;
  std::vector<Wt::Signals::connection> connections_;

  Wt::Signal<char> letterPushed_;

  void processButton(Wt::WPushButton *b);
  void processButtonPushed(const Wt::WKeyEvent &e, Wt::WPushButton *b);
};

#endif //LETTERS_WIDGET_H_
