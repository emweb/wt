// This may look like C code, but it's really -*- C++ -*-
/* 
 * Copyright (C) 2011 Emweb bv, Herent, Belgium
 *
 * See the LICENSE file for terms of use.
 */

#ifndef WORD_WIDGET_H_
#define WORD_WIDGET_H_

#include <Wt/WContainerWidget.h>

using namespace Wt;

class WordWidget : public WContainerWidget
{
public:
  WordWidget();

  std::wstring word() const { return word_; } 

  void init(const std::wstring &word);
  bool guess(wchar_t c);

  bool won();

private:
  std::vector<WText *>           wordLetters_;
  std::wstring                   word_;

  unsigned                       displayedLetters_;
};

#endif //WORD_WIDGET_H_
