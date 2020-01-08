/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "SessionInfo.h"

namespace Wt {

SessionInfo::SessionInfo(const std::string sessionId, pid_t childPId)
  : sessionId_(sessionId),
    childPId_(childPId)
{ }

}
