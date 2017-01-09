// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2016 Emweb bvba, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifndef FILEDROPAPPLICATION_H_
#define FILEDROPAPPLICATION_H_

#include <Wt/WApplication>
#include <Wt/WFileDropWidget>

using namespace Wt;
namespace Wt {
  class WContainerWidget;
}

class FileDropApplication : public WApplication
{
public:
  FileDropApplication(const WEnvironment& env);

private:
  WText *log_;
  WFileDropWidget *drop_;
  int nbUploads_;

  std::map<WFileDropWidget::File*, Wt::WContainerWidget*> icons_;

  void handleDrop(std::vector<WFileDropWidget::File *> files);
  void tooLarge(WFileDropWidget::File *file);
  void failed(WFileDropWidget::File *file);
  void saveFile(WFileDropWidget::File *file);
  void cancelUpload();
  void updateProgressListener();
  
  void showProgress(::uint64_t current, ::uint64_t total);
};


#endif
