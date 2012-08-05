// This may look like C code, but it's really -*- C++ -*-
/* 
 * Copyright (C) 2011 Emweb bvba, Heverlee, Belgium
 *
 * See the LICENSE file for terms of use.
 */

#ifndef WORD_WIDGET_H_
#define WORD_WIDGET_H_

#include <Wt/WContainerWidget>

class WordWidget : public Wt::WContainerWidget
{
public:
  WordWidget(Wt::WContainerWidget *parent = 0);

  std::wstring word() const { return word_; } 

  void init(const std::wstring &word);
  bool guess(wchar_t c);

  bool won();

private:
  std::vector<Wt::WText *>       wordLetters_;
  std::wstring                   word_;

  unsigned                       displayedLetters_;
};

#endif //WORD_WIDGET_H_
