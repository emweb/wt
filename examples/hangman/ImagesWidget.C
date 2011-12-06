/* 
 * Copyright (C) 2011 Emweb bvba, Heverlee, Belgium
 *
 * See the LICENSE file for terms of use.
 */

#include "ImagesWidget.h"

#include <Wt/WImage>

using namespace Wt;

const int ImagesWidget::HURRAY = -1;

ImagesWidget::ImagesWidget(int maxGuesses, WContainerWidget *parent)
  : WContainerWidget(parent)
{
  for (int i = 0; i <= maxGuesses; ++i) {
    std::string fname = "icons/hangman";
    fname += boost::lexical_cast<std::string>(i) + ".jpg";
    WImage *theImage = new WImage(fname, this);
    images_.push_back(theImage);

    // Although not necessary, we can avoid flicker (on konqueror)
    // by presetting the image size.
    theImage->resize(256, 256);
    theImage->hide();
  }

  WImage *hurray = new WImage("icons/hangmanhurray.jpg", this);
  hurray->hide();
  images_.push_back(hurray);

  image_ = 0;
  showImage(maxGuesses);
}

void ImagesWidget::showImage(int index)
{
  image(image_)->hide();
  image_ = index;
  image(image_)->show();
}

WImage *ImagesWidget::image(int index) const
{
  return index == HURRAY ? images_.back() : images_[index];
}
