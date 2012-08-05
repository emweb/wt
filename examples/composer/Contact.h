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
/*@{*/

/*! \brief An email contact.
 *
 * This widget is part of the %Wt composer example.
 */
struct Contact
{
  /*! \brief The contact name.
   */
  std::wstring name;

  /*! \brief The contact email address.
   */
  std::wstring email;

  /*! \brief Create a new contact.
   */
  Contact(const std::wstring name_, const std::wstring email_)
    : name(name_),
      email(email_)
  { }

  /*! \brief Get the typical single string form: "name" <email>
   */
  std::wstring formatted() const {
    return L'"' + name + L"\" <" + email + L">";
  }
};

/*@}*/

#endif // CONTACT_H_
