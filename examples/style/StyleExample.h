// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifndef STYLEEXAMPLE_H_
#define STYLEEXAMPLE_H_

#include <Wt/WContainerWidget.h>

namespace Wt {
  class WLineEdit;
  class WText;
}

using namespace Wt;

class RoundedWidget;

/**
 * \defgroup styleexample Style example
 */
/*@{*/

/*! \brief A demonstration of the RoundedWidget.
 *
 * This is the main class for the style example.
 */
class StyleExample : public WContainerWidget
{
public:
  /*! \brief Create a StyleExample.
   */
  StyleExample();

private:
  RoundedWidget *w_;
  WText *error_;

  WLineEdit *radius_, *r_, *g_, *b_;

  WLineEdit *createValidateLineEdit(int value, int min, int max);
  void updateStyle();
};

/*@}*/

#endif // STYLEEXAMPLE_H_
