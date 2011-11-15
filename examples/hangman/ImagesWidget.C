/* 
 * Copyright (C) 2011 Emweb bvba, Heverlee, Belgium
 *
 * See the LICENSE file for terms of use.
 */

#include "ImagesWidget.h"

#include <Wt/WImage>

using namespace Wt;

const int ImagesWidget::maxGuesses_ = 9;

ImagesWidget::ImagesWidget(WContainerWidget *parent)
{
  for (int i = 0; i <= maxGuesses_; ++i) {
    std::string fname = "icons/hangman";
    fname += boost::lexical_cast<std::string>(i) + ".jpg";
    WImage *theImage = new WImage(fname, this);
    hangmanImages_.push_back(theImage);
    
    // Although not necessary, we can avoid flicker (on konqueror)
    // by presetting the image size.
    theImage->resize(256, 256);
  }

  hurrayImage_ = new WImage("icons/hangmanhurray.jpg", this);

  reset();

  hangmanImages_[0]->hide();
  hangmanImages_.back()->show();
}

void ImagesWidget::reset()
{
  badGuesses_ = 0;

  hurrayImage_->hide();
  for(unsigned int i = 0; i < hangmanImages_.size(); ++i)
    hangmanImages_[i]->hide();
  hangmanImages_[0]->show();
}

void ImagesWidget::badGuess()
{
  if (badGuesses_ < (int)hangmanImages_.size() - 1) {
    hangmanImages_[badGuesses_]->hide();
    hangmanImages_[++badGuesses_]->show();
  }
}

bool ImagesWidget::gameOver()
{
  return badGuesses_ == maxGuesses_;
}

void ImagesWidget::hurray() 
{
  hangmanImages_[badGuesses_]->hide();
  hurrayImage_->show();
}
