/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "TimeSuggestions.h"

#include <Wt/WContainerWidget.h>

#include <cstdio>

using namespace Wt;

namespace {
  WSuggestionPopup::Options contactOptions
  = { "<b>",         // highlightBeginTag
      "</b>",        // highlightEndTag
      0,             // listSeparator
      " \n",        // whitespace
      "0",           // wordSeparators
      ""             // appendReplacedText
    };
}

TimeSuggestions::TimeSuggestions()
  : WSuggestionPopup(WSuggestionPopup::generateMatcherJS(contactOptions),
                     WSuggestionPopup::generateReplacerJS(contactOptions))
{
  for (unsigned i = 0; i < 24; i++) {
    char buffer [25];
    std::sprintf(buffer, "%02d", i);
    std::string h = buffer;
    
    addSuggestion(WString(h + ":00"));
    addSuggestion(WString(h + ":30"));
  }
}

void TimeSuggestions::addSuggestion(const WString& suggestion)
{
  WSuggestionPopup::addSuggestion(suggestion, suggestion);
}
