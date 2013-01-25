// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifndef FORMWIDGETS_H_
#define FORMWIDGETS_H_

#include "TopicWidget.h"

#include "Wt/WStandardItemModel"

class EventDisplayer;

class FormWidgets : public TopicWidget
{
public:
  FormWidgets();

  void populateSubMenu(Wt::WMenu *menu);

private:
  Wt::WWidget *introduction();
  Wt::WWidget *textEditors();
  Wt::WWidget *checkBox();
  Wt::WWidget *radioButton();
  Wt::WWidget *comboBox();
  Wt::WWidget *selectionBox();
  Wt::WWidget *autoComplete();
  Wt::WWidget *dateEntry();
  Wt::WWidget *inPlaceEdit();
  Wt::WWidget *slider();
  Wt::WWidget *progressBar();
  Wt::WWidget *fileUpload();
  Wt::WWidget *pushButton();
  Wt::WWidget *validation();
  Wt::WWidget *example();
};

#endif // FORMWIDGETS_H_
