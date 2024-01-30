// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2016 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifndef MYUPLOADRESOURCE_H_
#define MYUPLOADRESOURCE_H_

#include "MyUploadResource.h"

#include <Wt/WResource.h>
#include <Wt/WFileDropWidget.h>

/*! \brief A resource that handles incoming uploads by logging
 *
 * This simple upload resource handles incoming uploads by logging the
 * size, type and storage location.
 */
class MyUploadResource : public Wt::WResource
{
public:
  MyUploadResource(Wt::WFileDropWidget *fileDropWidget, Wt::WFileDropWidget::File *file);
  ~MyUploadResource() { beingDeleted(); }
protected:
  void handleRequest(const Wt::Http::Request &request,
                     Wt::Http::Response &response) override;
private:
  Wt::WFileDropWidget *parent_;
  Wt::WFileDropWidget::File *currentFile_;
};

#endif
