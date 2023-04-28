// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium
 *
 * See the LICENSE file for terms of use.
 */

#ifndef DICTIONARY_H_
#define DICTIONARY_H_

#include <string>

enum class Dictionary {
  English = 0,
  Dutch = 1
};

extern std::string randomWord(Dictionary dictionary);

#endif //DICTIONARY_H_
