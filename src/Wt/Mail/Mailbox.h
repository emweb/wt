// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_MAIL_MAILBOX_H_
#define WT_MAIL_MAILBOX_H_

#include <string>
#include <Wt/WString.h>

namespace Wt {
  namespace Mail {

/*! \class Mailbox Wt/Mail/Mailbox.h Wt/Mail/Mailbox.h
 *  \brief An email sender or recipient mailbox
 *
 * \ingroup mail
 */
class WT_API Mailbox
{
public:
  /*! \brief Default constructor.
   *
   * Creates an empty() mailbox.
   */
  Mailbox();

  /*! \brief Constructs a mailbox using an address.
   *
   * Constructs a mailbox which is only defined by an email address
   * (i.e. without a display name).
   */
  Mailbox(const std::string& address);

  /*! \brief Constructs a mailbox using an address and display name.
   */
  Mailbox(const std::string& address, const WT_USTRING& displayName);

  /*! \brief Returns whether the mailbox is empty.
   */
  bool empty() const { return address_.empty(); }

  void write(const std::string& header, std::ostream& out) const;

  /*! \brief Returns the email address.
   *
   * Returns "" if empty()
   */
  const std::string& address() const { return address_; }

  /*! \brief Returns the display name.
   */
  const WT_USTRING& displayName() const { return displayName_; }

private:
  std::string address_;
  WT_USTRING displayName_;
};

  }
}

#endif // WT_MAIL_MAILBOX_H_
