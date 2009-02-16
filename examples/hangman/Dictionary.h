/* this is a -*-C++-*- file
 * Copyright (C) 2005 Wim Dumon
 *
 * See the LICENSE file for terms of use.
 */

#ifndef DICTIONARY_H_
#define DICTIONARY_H_

#include <string>

enum Dictionary {
  DICT_EN = 0,
  DICT_NL = 1
};

extern std::wstring RandomWord(Dictionary dictionary);

#endif
