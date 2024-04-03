// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2016 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifndef FILEDROPAPPLICATION_H_
#define FILEDROPAPPLICATION_H_

#include <Wt/WApplication.h>
#include <Wt/WFileDropWidget.h>

namespace Wt {
  class WContainerWidget;
  class WTemplate;
}

class FileDropApplication : public Wt::WApplication
{
public:
  FileDropApplication(const Wt::WEnvironment& env);

private:
  Wt::WTemplate *tpl_;

  Wt::WText *progress_;
  Wt::WContainerWidget *log_;
  Wt::WFileDropWidget *drop_;

  std::map<Wt::WFileDropWidget::File*, Wt::WContainerWidget*> icons_;
  std::map<Wt::WFileDropWidget::File*, Wt::WFileDropWidget::Directory*> dirFiles_;

  void handleDrop(std::vector<Wt::WFileDropWidget::File *> files);
  void tooLarge(Wt::WFileDropWidget::File *file, ::uint64_t);
  void failed(Wt::WFileDropWidget::File *file);
  void saveFile(Wt::WFileDropWidget::File *file);
  void cancelUpload();
  void updateProgressListener();

  void showProgress(::uint64_t current, ::uint64_t total);
  Wt::WContainerWidget* getIcon(Wt::WFileDropWidget::File *file);
  void addLogLine(const std::string& msg);
};


#endif
