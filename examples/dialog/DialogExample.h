// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef DIALOG_EXAMPLE_H_
#define DIALOG_EXAMPLE_H_

#include <Wt/WApplication.h>
#include <Wt/WMessageBox.h>

using namespace Wt;

/**
 * \defgroup dialog Dialog example
 */
/*@{*/

/*! \brief An example illustrating usage of Dialogs
 */
class DialogExample : public WApplication
{
public:
  /*! \brief Create the example application.
   */
  DialogExample(const WEnvironment& env);

private:
  void messageBox1();
  void messageBox2();
  void messageBox3();
  void messageBox4();
  void custom();

  void messageBoxDone(StandardButton result);

  void setStatus(const WString& text);

  std::unique_ptr<WMessageBox> messageBox_;
  WText *status_;
};

/*@}*/

#endif // DIALOGEXAMPLE_H_
