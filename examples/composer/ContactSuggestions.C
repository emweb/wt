/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <boost/lexical_cast.hpp>

#include "ContactSuggestions.h"
#include "AddresseeEdit.h"
#include "Contact.h"

#include <Wt/WContainerWidget.h>

namespace {
  WSuggestionPopup::Options contactOptions
  = { "<b>",         // highlightBeginTag
      "</b>",        // highlightEndTag
      ',',           // listSeparator
      " \n",        // whitespace
      "-., \"@\n;", // wordSeparators
      ", "           // appendReplacedText
    };
}

ContactSuggestions::ContactSuggestions()
  : WSuggestionPopup(WSuggestionPopup::generateMatcherJS(contactOptions),
                     WSuggestionPopup::generateReplacerJS(contactOptions))
{ }

void ContactSuggestions::setAddressBook(const std::vector<Contact>& contacts)
{
  clearSuggestions();

  for(auto contact : contacts)
    addSuggestion(contact.formatted(), contact.formatted());
}
