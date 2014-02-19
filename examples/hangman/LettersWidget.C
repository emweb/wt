/* 
 * Copyright (C) 2011 Emweb bvba, Heverlee, Belgium
 *
 * See the LICENSE file for terms of use.
 */

#include "LettersWidget.h"

#include <Wt/WPushButton>
#include <Wt/WTable>
#include <Wt/WApplication>
#include <Wt/WEvent>

using namespace Wt;

LettersWidget::LettersWidget(WContainerWidget *parent)
  : WCompositeWidget(parent)
{
  setImplementation(impl_ = new WTable());

  impl_->resize(13*30, WLength::Auto);

  for (unsigned int i = 0; i < 26; ++i) {
    std::string c(1, 'A' + i);
    WPushButton *character = new WPushButton(c,
					     impl_->elementAt(i / 13, i % 13));
    letterButtons_.push_back(character);
    character->resize(WLength(30), WLength::Auto);

    character->clicked().connect
      (boost::bind(&LettersWidget::processButton, this, character));

	WApplication::instance()->globalKeyPressed().connect
	  (boost::bind(&LettersWidget::processButtonPushed, this, _1, character));
  }
}

void LettersWidget::processButton(WPushButton *b)
{
  b->disable();
  letterPushed_.emit(b->text().toUTF8()[0]);
}

void LettersWidget::processButtonPushed(WKeyEvent &e, WPushButton *b)
{
  if(isHidden())
	  return;

  if(e.key() == b->text().toUTF8()[0])
    processButton(b);
}

void LettersWidget::reset()
{
  for (unsigned int i = 0; i < letterButtons_.size(); ++i)
    letterButtons_[i]->enable();

  show();
}
