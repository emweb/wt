/* 
 * Copyright (C) 2011 Emweb bv, Herent, Belgium
 *
 * See the LICENSE file for terms of use.
 */

#include "LettersWidget.h"

#include <Wt/WPushButton.h>
#include <Wt/WTable.h>
#include <Wt/WApplication.h>
#include <Wt/WEvent.h>
#include <Wt/WAny.h>

using namespace Wt;

LettersWidget::LettersWidget()
  : WCompositeWidget()
{
  impl_ = new WTable();
  setImplementation(std::unique_ptr<WTable>(impl_));

  impl_->resize(13*30, WLength::Auto);

  for (unsigned int i = 0; i < 26; ++i) {
    std::string c(1, 'A' + i);
    WPushButton *character
        = impl_->elementAt(i / 13, i % 13)->addWidget(cpp14::make_unique<WPushButton>(c));
    letterButtons_.push_back(character);
    character->resize(WLength(30), WLength::Auto);

    character->clicked().connect
      (std::bind(&LettersWidget::processButton, this, character));

    connections_.push_back(WApplication::instance()->globalKeyPressed().connect
      (std::bind(&LettersWidget::processButtonPushed, this, std::placeholders::_1, character)));
  }
}

LettersWidget::~LettersWidget()
{
  for (auto &connection : connections_)
    connection.disconnect();
}

void LettersWidget::processButton(WPushButton *b)
{
  b->disable();
  letterPushed_.emit(b->text().toUTF8()[0]);
}

void LettersWidget::processButtonPushed(const WKeyEvent &e, WPushButton *b)
{
  if (isHidden())
    return;

  if(e.key() == static_cast<Key>(b->text().toUTF8()[0]))
    processButton(b);
}

void LettersWidget::reset()
{
  for (auto& letterButton : letterButtons_)
    letterButton->enable();

  show();
}
