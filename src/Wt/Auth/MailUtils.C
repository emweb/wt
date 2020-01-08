/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WLogger.h"
#include "Wt/Mail/Client.h"

namespace Wt {
  namespace Auth {
    namespace MailUtils {
      void sendMail(const Mail::Message &m) {
	Mail::Client client;
	client.connect();
	client.send(m);
      }
    }
  }
}
