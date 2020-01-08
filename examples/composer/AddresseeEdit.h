// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef ADDRESSEE_EDIT_H_
#define ADDRESSEE_EDIT_H_

#include <Wt/WTextArea.h>

#include "Contact.h"

using namespace Wt;

namespace Wt {
  class WTableCell;
}

class Label;

/**
 * @addtogroup composerexample
 */
//!@{

/*! \brief An edit field for an email addressee.
 *
 * This widget is part of the %Wt composer example. 
 */
class AddresseeEdit : public WTextArea
{
public:
  /*! \brief Create a new addressee edit with the given label.
   *
   * Constructs also a widget to hold the label in the labelParent.
   * The label will be hidden and shown together with this field.
   */
  AddresseeEdit(const WString& label, WContainerWidget *labelParent);

  /*! \brief Set a list of addressees.
   */
  void setAddressees(const std::vector<Contact>& contacts);

  /*! \brief Get a list of addressees
   */
  std::vector<Contact> addressees() const;

  //! Reimplement hide() and show() to also hide() and show() the label.
  virtual void setHidden(bool hidden, const WAnimation& animation);

private:
  //! The label associated with this edit.
  Label *label_;

  //! Parse the addressees into a list of contacts.
  bool parse(std::vector<Contact>& contacts) const;
};

//!@}


#endif // ADDRESSEE_EDIT_H_
