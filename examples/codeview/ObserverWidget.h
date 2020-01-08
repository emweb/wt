// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifndef OBSERVER_WIDGET_H_
#define OBSERVER_WIDGET_H_

#include <Wt/WContainerWidget.h>

#include "CodeSession.h"

class ObserverWidget : public Wt::WContainerWidget
{
public:
  ObserverWidget(const std::string& id);
  virtual ~ObserverWidget();

private:
  std::shared_ptr<CodeSession> session_;

  void updateBuffer(int buffer, CodeSession::BufferUpdate update);
  void insertBuffer(const CodeSession::Buffer& buffer, int index);
};

#endif // OBSERVER_WIDGET_H_
