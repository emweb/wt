// This may look like C code, but it's really -*- C++ -*-
/* 
 * Copyright (C) 2011 Emweb bvba, Heverlee, Belgium
 *
 * See the LICENSE file for terms of use.
 */

#ifndef IMAGES_WIDGET_H_
#define IMAGES_WIDGET_H_

#include <vector>

#include <Wt/WContainerWidget>

namespace Wt {
  class WImage;
}

class ImagesWidget : public Wt::WContainerWidget
{
public:
  ImagesWidget(Wt::WContainerWidget *parent = 0);

  int badGuesses() const { return badGuesses_; }

  void reset();
  void badGuess();
  void hurray();
  bool gameOver();

private:
  std::vector<Wt::WImage *>      hangmanImages_;
  Wt::WImage                    *hurrayImage_;

  int                            badGuesses_;
  const static int               maxGuesses_;
};

#endif //IMAGES_WIDGET_H_
