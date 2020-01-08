// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifndef FORMEXAMPLE_H_
#define FORMEXAMPLE_H_

#include <Wt/WContainerWidget.h>

namespace Wt {
  class WText;
}

using namespace Wt;

/**
 * \defgroup formexample Form example
 */
/*@{*/

/*!\brief Main widget for the %Form example.
 *
 * This class demonstrates, next instantiating the form itself,
 * handling of different languages.
 */
class FormExample : public WContainerWidget
{
public:
  /*!\brief Instantiate a new form example.
   */
  FormExample();

private:
  std::vector<WText *> languageSelects_;

  /*!\brief Change the language.
   */
  void changeLanguage(WText *t);

  void setLanguage(const std::string lang);
};

/*@}*/

#endif // FORMEXAMPLE_H_
