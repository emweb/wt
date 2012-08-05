/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "MyCaptcha.h"

using namespace Wt;

MyCaptcha::MyCaptcha(WContainerWidget* parent, 
		     const int width, const int height)
  : WContainerWidget(parent),
    completed_(this)
{
  setStyleClass("captcha");
		
  captchaMessage_ = new WText(this);

  shapesWidget_ = new ShapesWidget(this);
  shapesWidget_->resize(width, height);

  shapesWidget_->clicked().connect(this, &MyCaptcha::handleClick);

  regenerate();
}
  
void MyCaptcha::handleClick(const WMouseEvent& me)
{
  if (shapesWidget_->correctlyClicked(me)) 
    completed_.emit();
  else
    regenerate();
}

void MyCaptcha::regenerate()
{
  shapesWidget_->initShapes();
  shapesWidget_->update();
  captchaMessage_->setText(tr("captcha.message")
			   .arg(shapesWidget_->selectedColor())
			   .arg(shapesWidget_->selectedShape()));
}
