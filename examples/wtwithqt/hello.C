/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 * 
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include <iostream>

#include <Wt/WBreak.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WLineEdit.h>
#include <Wt/WPushButton.h>
#include <Wt/WText.h>

#include "HelloApplication.h"
#include "QtObject.h"

// Needed when using WQApplication with Qt eventloop = true
//#include <QCoreApplication>

using namespace Wt;

Dictionary::Dictionary(const WEnvironment& env)
  : WQApplication(env /*,  true */)
{
  /*
   * Note: do not create any Qt objects from here. Initialize your
   * application from within the virtual create() method.
   */
}

void Dictionary::create()
{
  setTitle("Hello world");

  root()->addWidget(std::make_unique<WText>("Your name, please? "));
  nameEdit_ = root()->addWidget(std::make_unique<WLineEdit>());
  nameEdit_->setFocus();

  WPushButton *b = root()->addWidget(std::make_unique<WPushButton>("Greet me."));
  b->setMargin(5, Side::Left);

  root()->addWidget(std::make_unique<WBreak>());

  greeting_ = root()->addWidget(std::make_unique<WText>());

  b->clicked().connect(this, &Dictionary::propagateGreet);
  nameEdit_->enterPressed().connect(this, &Dictionary::propagateGreet);

  qtSender_ = std::make_unique<QtObject>(this);
  qtReceiver_ = std::make_unique<QtObject>(this);

  QObject::connect(qtSender_.get(), SIGNAL(greet(const QString&)),
		   qtReceiver_.get(), SLOT(doGreet(const QString&)));
}

void Dictionary::destroy()
{
  /*
   * Note: Delete any Qt object from here.
   */
  qtSender_.reset();
  qtReceiver_.reset();
}

void Dictionary::propagateGreet()
{
  qtSender_->passGreet(toQString(nameEdit_->text()));
}

void Dictionary::doGreet(const QString& qname)
{
  greeting_->setText("Hello there, " + toWString(qname));
}

std::unique_ptr<WApplication> createApplication(const WEnvironment& env)
{
  return std::make_unique<Dictionary>(env);
}

int main(int argc, char **argv)
{
  // Needed for Qt's eventloop threads to work
  //QCoreApplication app(argc, argv);

  return WRun(argc, argv, &createApplication);
}

