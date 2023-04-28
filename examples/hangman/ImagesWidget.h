// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium
 *
 * See the LICENSE file for terms of use.
 */

#ifndef IMAGES_WIDGET_H_
#define IMAGES_WIDGET_H_

#include <Wt/WContainerWidget.h>

#include <vector>

class ImagesWidget : public Wt::WContainerWidget
{
public:
  static const int HURRAY;

  explicit ImagesWidget(int maxGuesses);

  /*
   * 0 - maxGuesses: corresponds to 0 up to maxGuesses guesses
   *         HURRAY: when won
   */
  void showImage(int index);
  int currentImage() const { return image_; }

private:
  std::vector<Wt::WImage*> images_;
  int image_;

  Wt::WImage *image(int index) const;
};

#endif // IMAGES_WIDGET_H_
