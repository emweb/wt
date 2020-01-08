// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef CONTACT_SUGGESTIONS_H_
#define CONTACT_SUGGESTIONS_H_

#include <Wt/WSuggestionPopup.h>

class AddresseeEdit;
struct Contact;

using namespace Wt;

/**
 * @addtogroup composerexample
 */
/*@{*/

/*! \brief A suggestion popup suggesting contacts from an addressbook.
 *
 * This popup provides suggestions from a list of contact, by
 * matching parts of the name or email adress with the current
 * value being edited. It also supports editing a list of email
 * addresses.
 *
 * The popup is only available when JavaScript is available, and
 * is implemented entirely on the client-side.
 */
class ContactSuggestions : public WSuggestionPopup
{
public:
  /*! \brief Create a new ContactSuggestions popup.
   */
  ContactSuggestions();

  /*! \brief Set the address book.
   */
  void setAddressBook(const std::vector<Contact>& contacts);

private:
};

/*@}*/

#endif // CONTACT_SUGGESTIONS_H_
