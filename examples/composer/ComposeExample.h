// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef COMPOSE_EXAMPLE_H_
#define COMPOSE_EXAMPLE_H_

#include <Wt/WContainerWidget.h>

using namespace Wt;

class Composer;

namespace Wt {
  class WTreeNode;
}

/**
 * \defgroup composerexample Composer example
 */
/*@{*/

/*! \brief Main widget of the %Composer example.
 */
class ComposeExample : public WContainerWidget
{
public:
  /*! \brief create a new Composer example.
   */
  ComposeExample();

private:
  Composer *composer_;
  WContainerWidget *details_;

  void send();
  void discard();
};

/*@}*/

#endif // COMPOSE_EXAMPLE_H_
