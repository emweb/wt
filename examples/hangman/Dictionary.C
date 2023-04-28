/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium
 *
 * See the LICENSE file for terms of use.
 */
#include "Dictionary.h"

#include <Wt/WApplication.h>

#include <fstream>
#include <iostream>
#include <random>

std::string randomWord(Dictionary dictionary)
{
  std::ifstream dict;
  if (dictionary == Dictionary::Dutch) {
    dict.open(Wt::WApplication::appRoot() + "dict-nl.txt");
  } else { // english is default
    dict.open(Wt::WApplication::appRoot() + "dict.txt");
  }

  std::string retval;
  int numwords = 0;
  while (dict) {
    std::getline(dict, retval);
    numwords++;
  }
  dict.clear();
  dict.seekg(0);

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> distribution(0, numwords);
  int selection = distribution(gen);

  while (selection--) {
    std::getline(dict, retval);
  }
  std::getline(dict, retval);
  for (unsigned int i = 0; i < retval.size(); ++i) {
    if (retval[i] < 'A' || retval[i] > 'Z') {
      std::cerr << "word " << retval << " contains illegal data at pos " << i << '\n';
    }
  }

  return retval;
}
