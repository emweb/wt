// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium
 *
 * See the LICENSE file for terms of use.
 */

#ifndef DICTIONARY_H_
#define DICTIONARY_H_

#include <string>

using namespace Wt;

enum Dictionary {
  DICT_EN = 0,
  DICT_NL = 1
};

extern std::wstring RandomWord(Dictionary dictionary);

#endif //DICTIONARY_H_
