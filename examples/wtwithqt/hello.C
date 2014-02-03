/*
 * Copyright (C) 2008 Emweb bvba, Heverlee, Belgium.
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

#include <Wt/WBreak>
#include <Wt/WContainerWidget>
#include <Wt/WLineEdit>
#include <Wt/WPushButton>
#include <Wt/WText>

#include "HelloApplication.h"
#include "QtObject.h"

// Needed when using WQApplication with Qt eventloop = true
//#include <QApplication>

using namespace Wt;

Dictionary::Dictionary(const WEnvironment& env)
  : WQApplication(env /*, true */)
{
  /*
   * Note: do not create any Qt objects from here. Initialize your
   * application from within the virtual create() method.
   */
}

void Dictionary::create()
{
  setTitle("Hello world");

  root()->addWidget(new WText("Your name, please ? "));
  nameEdit_ = new WLineEdit(root());
  nameEdit_->setFocus();

  WPushButton *b = new WPushButton("Greet me.", root());
  b->setMargin(5, Left);

  root()->addWidget(new WBreak());

  greeting_ = new WText(root());

  b->clicked().connect(this, &Dictionary::propagateGreet);
  nameEdit_->enterPressed().connect(this, &Dictionary::propagateGreet);

  qtSender_ = new QtObject(this);
  qtReceiver_ = new QtObject(this);

  QObject::connect(qtSender_, SIGNAL(greet(const QString&)),
		   qtReceiver_, SLOT(doGreet(const QString&)));
}

void Dictionary::destroy()
{
  /*
   * Note: Delete any Qt object from here.
   */
  delete qtSender_;
  delete qtReceiver_;
}

void Dictionary::propagateGreet()
{
  qtSender_->passGreet(toQString(nameEdit_->text()));
}

void Dictionary::doGreet(const QString& qname)
{
  greeting_->setText("Hello there, " + toWString(qname));
}

WApplication *createApplication(const WEnvironment& env)
{
  return new Dictionary(env);
}

int main(int argc, char **argv)
{
  // Needed for Qt's eventloop threads to work
  //QApplication app(argc, argv);

  return WRun(argc, argv, &createApplication);
}

