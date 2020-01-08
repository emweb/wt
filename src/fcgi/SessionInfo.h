/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
// This may look like C code, but it's really -*- C++ -*-
#ifndef WT_SESSIONINFO_H_
#define WT_SESSIONINFO_H_

#include <stdio.h>
#include <sys/types.h>
#include <string>

namespace Wt {

class SessionInfo
{
public:
  SessionInfo(const std::string sessionId, pid_t childPId);

  const std::string sessionId() const { return sessionId_; }
  pid_t childPId() const { return childPId_; }

private:
  std::string sessionId_;
  pid_t       childPId_;
};

}

#endif // WT_SESSIONINFO_H_
