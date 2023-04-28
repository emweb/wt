// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium
 *
 * See the LICENSE file for terms of use.
 */

#ifndef WORD_WIDGET_H_
#define WORD_WIDGET_H_

#include <Wt/WContainerWidget.h>

class WordWidget : public Wt::WContainerWidget
{
public:
  WordWidget();

  std::string word() const { return word_; }

  void init(const std::string &word);
  bool guess(char c);

  bool won();

private:
  std::vector<Wt::WText*> wordLetters_;
  std::string word_;

  unsigned displayedLetters_ = 0;
};

#endif //WORD_WIDGET_H_
