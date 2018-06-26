// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifndef FORMWIDGETS_H_
#define FORMWIDGETS_H_

#include "TopicWidget.h"

#include "Wt/WStandardItemModel.h"

class EventDisplayer;

class FormWidgets : public TopicWidget
{
public:
  FormWidgets();

  void populateSubMenu(Wt::WMenu *menu);

private:
  std::unique_ptr<WWidget> introduction();
  std::unique_ptr<WWidget> textEditors();
  std::unique_ptr<WWidget> checkBox();
  std::unique_ptr<WWidget> radioButton();
  std::unique_ptr<WWidget> comboBox();
  std::unique_ptr<WWidget> selectionBox();
  std::unique_ptr<WWidget> autoComplete();
  std::unique_ptr<WWidget> dateEntry();
  std::unique_ptr<WWidget> inPlaceEdit();
  std::unique_ptr<WWidget> slider();
  std::unique_ptr<WWidget> progressBar();
  std::unique_ptr<WWidget> fileUpload();
  std::unique_ptr<WWidget> pushButton();
  std::unique_ptr<WWidget> validation();
  std::unique_ptr<WWidget> example();
};

#endif // FORMWIDGETS_H_
