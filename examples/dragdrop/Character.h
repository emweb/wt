// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef CHARACTER_H_
#define CHARACTER_H_

#include <Wt/WText.h>

using namespace Wt;

/**
 * @addtogroup dragexample
 */
/*@{*/

/*! \brief A Matrix character that takes red and/or blue pills.
 *
 * The Character class demonstrates how to accept and react to drop
 * events.
 */
class Character : public WText
{
public:
  /*! \brief Create a new character with the given name.
   */
  Character(const std::string& name);

  /*! \brief React to a drop event.
   */
  void dropEvent(WDropEvent event);

private:
  //! The name
  std::string name_;

  //! The current number of red pills.
  int redDrops_;

  //! The current number of blue pills.
  int blueDrops_;
};

/*@}*/

#endif // CHARACTER_H_
