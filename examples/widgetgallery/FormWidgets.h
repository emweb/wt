// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifndef FORMWIDGETS_H_
#define FORMWIDGETS_H_

#include "TopicWidget.h"

#include "Wt/WStandardItemModel.h"

#include <memory>

class EventDisplayer;

class FormWidgets : public TopicWidget
{
public:
  FormWidgets();

  void populateSubMenu(Wt::WMenu *menu);

private:
  std::unique_ptr<Wt::WWidget> introduction();
  std::unique_ptr<Wt::WWidget> textEditors();
  std::unique_ptr<Wt::WWidget> checkBox();
  std::unique_ptr<Wt::WWidget> radioButton();
  std::unique_ptr<Wt::WWidget> comboBox();
  std::unique_ptr<Wt::WWidget> selectionBox();
  std::unique_ptr<Wt::WWidget> autoComplete();
  std::unique_ptr<Wt::WWidget> dateEntry();
  std::unique_ptr<Wt::WWidget> inPlaceEdit();
  std::unique_ptr<Wt::WWidget> slider();
  std::unique_ptr<Wt::WWidget> progressBar();
  std::unique_ptr<Wt::WWidget> fileUpload();
  std::unique_ptr<Wt::WWidget> pushButton();
  std::unique_ptr<Wt::WWidget> validation();
  std::unique_ptr<Wt::WWidget> example();
};

#endif // FORMWIDGETS_H_
