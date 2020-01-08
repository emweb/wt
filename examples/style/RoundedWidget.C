/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WCssDecorationStyle.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WImage.h>

#include "RoundedWidget.h"
#include "CornerImage.h"

RoundedWidget::RoundedWidget(WFlags<Corner> corners)
  : WCompositeWidget(),
    backgroundColor_(WColor(0xD4,0xDD,0xFF)),
    surroundingColor_(WColor(0xFF,0xFF,0xFF)),
    radius_(10),
    corners_(corners)
{
  std::unique_ptr<WContainerWidget> impl(cpp14::make_unique<WContainerWidget>());
  impl_ = impl.get();
  setImplementation(std::move(impl));
  contents_ = impl_->addWidget(cpp14::make_unique<WContainerWidget>());

  create();
}

void RoundedWidget::create()
{
  std::array<std::unique_ptr<CornerImage>, 4> images;

  if (corners_.test(Corner::TopLeft)) {
    images[0] = Wt::cpp14::make_unique<CornerImage>(
				Corner::TopLeft, backgroundColor_,
				surroundingColor_, radius_);
    images[0]->setPositionScheme(PositionScheme::Absolute);
    images[0]->setMargin(0);
  }

  if (corners_.test(Corner::TopRight))
    images[1] = Wt::cpp14::make_unique<CornerImage>(
				Corner::TopRight, backgroundColor_,
				surroundingColor_, radius_);

  if (corners_.test(Corner::BottomLeft)) {
    images[2] = Wt::cpp14::make_unique<CornerImage>(
				Corner::BottomLeft, backgroundColor_,
				surroundingColor_, radius_);
    images[2]->setPositionScheme(PositionScheme::Absolute);
    images[2]->setMargin(0);
  }

  if (corners_.test(Corner::BottomRight))
    images[3] = Wt::cpp14::make_unique<CornerImage>(
				Corner::BottomRight, backgroundColor_,
				surroundingColor_, radius_);

  for (int i = 0; i < 4; ++i)
    images_[i] = images[i].get();

  /*
   * At the top: an image (top left corner) inside
   * a container widget with background image top right.
   */
  std::unique_ptr<WContainerWidget> top(cpp14::make_unique<WContainerWidget>());
  top_ = top.get();
  top_->resize(WLength::Auto, radius_);
  top_->setPositionScheme(PositionScheme::Relative);
  if (images_[1]) {
    top_->decorationStyle().setBackgroundImage(images_[1]->imageLink(),
                                               WFlags<Orientation>(),
                                               Side::Top | Side::Right);
    top_->addChild(std::move(images[1]));
  }
  if (images_[0]){
    top_->addWidget(std::move(images[0]));
  }
  impl_->insertBefore(std::move(top), contents_); // insert top before the contents

  /*
   * At the bottom: an image (bottom left corner) inside
   * a container widget with background image bottom right.
   */
  std::unique_ptr<WContainerWidget> bottom(cpp14::make_unique<WContainerWidget>());
  bottom_ = bottom.get();
  bottom_->setPositionScheme(PositionScheme::Relative);
  bottom_->resize(WLength::Auto, radius_);
  if (images_[3]) {
    bottom_->decorationStyle().setBackgroundImage(images_[3]->imageLink(),
                                                  WFlags<Orientation>(),
                                                  Side::Bottom | Side::Right);
    bottom_->addChild(std::move(images[3]));
  }
  if (images_[2]){
    bottom_->addWidget(std::move(images[2]));
  }
  impl_->addWidget(std::move(bottom));

  decorationStyle().setBackgroundColor(backgroundColor_);

  contents_->setMargin(WLength(radius_), Side::Left | Side::Right);
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
        .setBackgroundImage(images_[1]->imageLink(),
			    WFlags<Orientation>(),
			    Side::Top | Side::Right);
  }

  if (images_[3]) {
    images_[3]->setHidden(!how);
    if (!how)
      bottom_->decorationStyle().setBackgroundImage("");
    else
      bottom_->decorationStyle()
        .setBackgroundImage(images_[3]->imageLink(),
			    WFlags<Orientation>(),
			    Side::Top | Side::Right);
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
    top_->decorationStyle().setBackgroundImage(images_[1]->imageLink(),
                                               WFlags<Orientation>(),
                                               Side::Top | Side::Right);
  if (images_[3])
    bottom_->decorationStyle().setBackgroundImage(images_[3]->imageLink(),
                                                  WFlags<Orientation>(),
                                                  Side::Bottom | Side::Right);

  top_->resize(WLength::Auto, radius_);
  bottom_->resize(WLength::Auto, radius_);
  contents_->setMargin(radius_, Side::Left | Side::Right);

  decorationStyle().setBackgroundColor(backgroundColor_);
}
