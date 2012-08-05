/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifndef TIME_SUGGESTIONS_H_
#define TIME_SUGGESTIONS_H_

#include <Wt/WSuggestionPopup>

#include <string>

class TimeSuggestions : public Wt::WSuggestionPopup
{
 public:
  TimeSuggestions(Wt::WContainerWidget *parent = 0);
  
 private:
  void addSuggestion(const Wt::WString& suggestion);
};

#endif //TIME_SUGGESTIONS_H_
