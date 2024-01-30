// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2016 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifndef MYFILEDROPWIDGET_H_
#define MYFILEDROPWIDGET_H_

#include "MyUploadResource.h"

#include <Wt/WFileDropWidget.h>

namespace Wt {
  class WResource;
}

/*! \brief A file drop widget that transfers to a dummy endpoint
 * 
 * This file drop widget replaces the default upload mechanism with
 * a custom one, which logs the name and type to the JS console and
 * sends the data to a dummy endpoint at httpbin.org.
 */
class MyFileDropWidget : public Wt::WFileDropWidget
{
public:
  MyFileDropWidget();
protected:
  std::unique_ptr<Wt::WResource> uploadResource() { return std::make_unique<MyUploadResource>(this, currentFile()); };
};

#endif
