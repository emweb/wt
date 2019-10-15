// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef CONTACT_H_
#define CONTACT_H_

/**
 * @addtogroup composerexample
 */
//!@{

/*! \brief An email contact.
 *
 * This widget is part of the %Wt composer example.
 */
struct Contact
{
  /*! \brief The contact name.
   */
  std::u32string name;

  /*! \brief The contact email address.
   */
  std::u32string email;

  /*! \brief Create a new contact.
   */
  Contact(const std::u32string name_, const std::u32string email_)
    : name(name_),
      email(email_)
  { }

  /*! \brief Get the typical single string form: "name" <email>
   */
  std::u32string formatted() const {
    return U"\"" + name + U"\" <" + email + U">";
  }
};

//!@}

#endif // CONTACT_H_
