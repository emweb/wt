// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2016 Emweb bvba, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifndef FILEDROPAPPLICATION_H_
#define FILEDROPAPPLICATION_H_

#include <Wt/WApplication>

using namespace Wt;

namespace Wt {
  class WFileDropWidget;
}

class FileDropApplication : public WApplication
{
public:
  FileDropApplication(const WEnvironment& env);

private:
  WText *log_;
  WFileDropWidget *drop_;

  void saveFile();
  void fileTooBig();
  void connectFileUpload();
};


#endif
