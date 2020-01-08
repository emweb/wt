/*
 * Copyright (C) 2016 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "StringUtils.h"

namespace Wt {
  namespace Utils {

void split(SplitSet& tokens, const std::string &in, const char *sep,
	   bool compress_adjacent_tokens)
{
    boost::split(tokens, in, boost::is_any_of(sep),
		 compress_adjacent_tokens?
		 boost::algorithm::token_compress_on:
		 boost::algorithm::token_compress_off);
}

std::string splitEntryToString(SplitEntry se)
{
#ifndef WT_TARGET_JAVA
  return std::string(se.begin(), se.end());
#else
  return se;
#endif
}

  }
}
