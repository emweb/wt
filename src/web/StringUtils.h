// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_STRING_UTILS_H_
#define WT_STRING_UTILS_H_

#include <boost/algorithm/string.hpp>
#include <vector>
#include <set>

namespace Wt { namespace Utils {

#ifndef WT_TARGET_JAVA
typedef boost::iterator_range<std::string::const_iterator> SplitEntry;
#else
typedef std::string SplitEntry;
#endif

typedef std::vector<SplitEntry> SplitVector;
typedef std::set<SplitEntry> SplitSet;

// Splits a string in a set of strings, on every given token
extern void split(SplitSet& tokens,
		  const std::string &in, const char *sep,
		  bool compress_adjacent_tokens);

extern std::string splitEntryToString(SplitEntry se);

}}

#endif // WT_STRING_UTILS_H_
