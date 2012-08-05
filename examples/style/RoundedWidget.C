/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WCssDecorationStyle>
#include <Wt/WContainerWidget>
#include <Wt/WImage>

#include "RoundedWidget.h"
#include "CornerImage.h"

RoundedWidget::RoundedWidget(int corners, WContainerWidget *parent)
  : WCompositeWidget(parent),
    backgroundColor_(WColor(0xD4,0xDD,0xFF)),
    surroundingColor_(WColor(0xFF,0xFF,0xFF)),
    radius_(10),
    corners_(corners)
{
  setImplementation(impl_ = new WContainerWidget());
  contents_ = new WContainerWidget(impl_);

  create();
}

RoundedWidget::~RoundedWidget()
{
  if (images_[1])
    delete images_[1];
  if (images_[3])
    delete images_[3];
}

void RoundedWidget::create()
{
  if (corners_ & TopLeft) {
    images_[0] = new CornerImage(CornerImage::TopLeft, backgroundColor_,
				 surroundingColor_, radius_);
    images_[0]->setPositionScheme(Absolute);
    images_[0]->setMargin(0);
  } else
    images_[0] = 0;

  if (corners_ & TopRight)
    images_[1] = new CornerImage(CornerImage::TopRight, backgroundColor_,
				 surroundingColor_, radius_);
  else
    images_[1] = 0;

  if (corners_ & BottomLeft) {
    images_[2] = new CornerImage(CornerImage::BottomLeft, backgroundColor_,
				 surroundingColor_, radius_);
    images_[2]->setPositionScheme(Absolute);
    images_[2]->setMargin(0);
  } else
    images_[2] = 0;

  if (corners_ & BottomRight)
    images_[3] = new CornerImage(CornerImage::BottomRight, backgroundColor_,
				 surroundingColor_, radius_);
  else
    images_[3] = 0;

  /*
   * At the top: an image (top left corner) inside
   * a container widget with background image top right.
   */
  top_ = new WContainerWidget();
  top_->resize(WLength::Auto, radius_);
  top_->setPositionScheme(Relative);
  if (images_[1])
    top_->decorationStyle().setBackgroundImage(images_[1]->imageRef(),
					       WCssDecorationStyle::NoRepeat,
					       Top | Right);
  if (images_[0])
    top_->addWidget(images_[0]);
  impl_->insertBefore(top_, contents_); // insert top before the contents

  /*
   * At the bottom: an image (bottom left corner) inside
   * a container widget with background image bottom right.
   */
  bottom_ = new WContainerWidget();
  bottom_->setPositionScheme(Relative);
  bottom_->resize(WLength::Auto, radius_);
  if (images_[3])
    bottom_->decorationStyle().setBackgroundImage(images_[3]->imageRef(),
						  WCssDecorationStyle::NoRepeat,
						  Bottom | Right);
  if (images_[2])
    bottom_->addWidget(images_[2]);
  impl_->addWidget(bottom_);

  decorationStyle().setBackgroundColor(backgroundColor_);

  contents_->setMargin(WLength(radius_), Left | Right);
}

void RoundedWidget::setBackgroundColor(WColor color)
{
  backgroundColor_ = color;
  adjust();
}

void RoundedWidget::setSurroundingColor(WColor color)
{
  surroundingColor_ = color;
  adjust();
}

void RoundedWidget::setCornerRadius(int radius)
{
  radius_ = radius;
  adjust();
}

void RoundedWidget::enableRoundedCorners(bool how)
{
  if (images_[0]) images_[0]->setHidden(!how);
  if (images_[2]) images_[2]->setHidden(!how);

  if (images_[1]) {
    images_[1]->setHidden(!how);
    if (!how)
      top_->decorationStyle().setBackgroundImage("");
    else
      top_->decorationStyle()
	.setBackgroundImage(images_[1]->imageRef(),
			    WCssDecorationStyle::NoRepeat,
			    Top | Right);
  }

  if (images_[3]) {
    images_[3]->setHidden(!how);
    if (!how)
      bottom_->decorationStyle().setBackgroundImage("");
    else
      bottom_->decorationStyle()
	.setBackgroundImage(images_[3]->imageRef(),
			    WCssDecorationStyle::NoRepeat,
			    Top | Right);
  }
}

void RoundedWidget::adjust()
{
  if (images_[0] && !images_[0]->isHidden()) images_[0]->setRadius(radius_);
  if (images_[1] && !images_[1]->isHidden()) images_[1]->setRadius(radius_);
  if (images_[2] && !images_[2]->isHidden()) images_[2]->setRadius(radius_);
  if (images_[3] && !images_[3]->isHidden()) images_[3]->setRadius(radius_);

  if (images_[0] && !images_[0]->isHidden())
    images_[0]->setForeground(backgroundColor_);
  if (images_[1] && !images_[1]->isHidden())
    images_[1]->setForeground(backgroundColor_);
  if (images_[2] && !images_[2]->isHidden())
    images_[2]->setForeground(backgroundColor_);
  if (images_[3] && !images_[3]->isHidden())
    images_[3]->setForeground(backgroundColor_);

  if (images_[1]) 
    top_->decorationStyle().setBackgroundImage(images_[1]->imageRef(),
					       WCssDecorationStyle::NoRepeat,
					       Top | Right);
  if (images_[3])
    bottom_->decorationStyle().setBackgroundImage(images_[3]->imageRef(),
						  WCssDecorationStyle::NoRepeat,
						  Bottom | Right);

  top_->resize(WLength::Auto, radius_);
  bottom_->resize(WLength::Auto, radius_);
  contents_->setMargin(radius_, Left | Right);

  decorationStyle().setBackgroundColor(backgroundColor_);
}
