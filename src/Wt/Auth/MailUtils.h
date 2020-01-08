// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifndef WT_MAIL_UTILS_H_
#define WT_MAIL_UTILS_H_

#include "Wt/Mail/Message.h"
#include <Wt/WDllDefs.h>

namespace Wt {
  namespace Auth {
    namespace MailUtils {

      WT_API extern void sendMail(const Mail::Message &m);
    }
  }
}

#endif // WT_MAIL_UTILS_H_
