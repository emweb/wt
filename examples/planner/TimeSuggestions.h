/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifndef TIME_SUGGESTIONS_H_
#define TIME_SUGGESTIONS_H_

#include <Wt/WSuggestionPopup.h>

using namespace Wt;

class TimeSuggestions : public WSuggestionPopup
{
 public:
  TimeSuggestions();
  
 private:
  void addSuggestion(const WString& suggestion);
};

#endif //TIME_SUGGESTIONS_H_
