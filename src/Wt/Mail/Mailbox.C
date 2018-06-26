/*
 * Copyright (C) 2011 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Mailbox.h"
#include "Message.h"

#include <iostream>

namespace Wt {
  namespace Mail {

Mailbox::Mailbox()
{ }

Mailbox::Mailbox(const std::string& address)
  : address_(address)
{ }

Mailbox::Mailbox(const std::string& address, const WString& displayName)
  : address_(address),
    displayName_(displayName)
{ }

void Mailbox::write(const std::string& header, std::ostream& out) const
{
  out << header << ": ";
  if (!displayName_.empty()) {
    Message::encodeWord(displayName_, out, true);
    out << " ";
  }
  out << "<" << address_ << ">\r\n";
}

  }
}
