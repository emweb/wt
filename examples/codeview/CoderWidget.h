// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef CODER_WIDGET_H_
#define CODER_WIDGET_H_

#include <Wt/WContainerWidget.h>

#include "CodeSession.h"

class BufferEditorWidget;

class CoderWidget : public Wt::WContainerWidget
{
public:
  CoderWidget();
  virtual ~CoderWidget();

private:
  std::shared_ptr<CodeSession> session_;

  Wt::WContainerWidget *buffers_;
  Wt::WText *observerCount_;

  void addBuffer();
  void insertBuffer(int index);
  void changed(BufferEditorWidget *editor);
  void sessionChanged();
};

#endif // CODER_WIDGET_H_
