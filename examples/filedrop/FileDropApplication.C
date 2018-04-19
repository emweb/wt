/*
 * Copyright (C) 2016 Emweb bvba, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "FileDropApplication.h"

#include <Wt/WContainerWidget>
#include <Wt/WLineEdit>
#include <Wt/WPushButton>
#include <Wt/WText>
#include <Wt/WFileDropWidget>
#include "Wt/Utils"

#include <boost/algorithm/string/predicate.hpp>

#include <iostream>
#include <fstream>

#define MULTILINE(...) #__VA_ARGS__

using namespace Wt;
using namespace boost::algorithm;

static const std::string UPLOAD_FOLDER = "./uploaded/";
static const int MAX_FILES = 36;

static const std::string gzipFilter =
#include "createFilterFn.js"
  ;

FileDropApplication::FileDropApplication(const WEnvironment& env)
  : WApplication(env),
    nbUploads_(0)
{
  setTitle("File Drop Example");
  useStyleSheet("style.css");

  require("https://cdn.jsdelivr.net/pako/1.0.3/pako.min.js");

  new WText("<h1>Try dropping a file in the widget below</h1>", root());

  drop_ = new WFileDropWidget(root());

  drop_->setDropIndicationEnabled(true);
  // drop_->setGlobalDropEnabled(true);
  
  std::vector<std::string> filterFnImports;
  filterFnImports.push_back("https://cdn.jsdelivr.net/pako/1.0.3/pako.min.js");
  drop_->setJavaScriptFilter(gzipFilter, 1*512*1024, filterFnImports);

  drop_->drop().connect(this, &FileDropApplication::handleDrop);
  drop_->newUpload().connect(this,&FileDropApplication::updateProgressListener);
  drop_->uploaded().connect(this, &FileDropApplication::saveFile);
  drop_->uploadFailed().connect(this, &FileDropApplication::failed);
  drop_->tooLarge().connect(this, &FileDropApplication::tooLarge);
  
  log_ = new WText(root());
  log_->setInline(false);
  log_->setTextFormat(XHTMLText);

  WPushButton *abort = new WPushButton("Abort current upload", root());
  abort->clicked().connect(this, &FileDropApplication::cancelUpload);

  new WLineEdit(root()); // to check responsiveness
}

void FileDropApplication::handleDrop(std::vector<WFileDropWidget::File *> files)
{
  for (unsigned i=0; i < files.size(); i++) {
    WFileDropWidget::File *file = files[i];
    if (nbUploads_ >= MAX_FILES) {
      drop_->cancelUpload(file);
      continue;
    }

    file->setFilterEnabled(ends_with(file->clientFileName(), ".fastq"));

    WContainerWidget *block = new WContainerWidget(drop_);
    block->setToolTip(file->clientFileName() + " [" + file->mimeType() + "]");
    block->addStyleClass("upload-block");

    icons_[file] = block;
    nbUploads_++;
  }
  
  if (nbUploads_ >= MAX_FILES) {
    log_->setText("That's enough ...");
    drop_->setAcceptDrops(false);
  }
}

void FileDropApplication::cancelUpload()
{
  if (drop_->uploads().size() == drop_->currentIndex())
    return;
  
  WFileDropWidget::File *currentFile = drop_->uploads()[drop_->currentIndex()];
  drop_->cancelUpload(currentFile);
  icons_[currentFile]->addStyleClass("cancelled");
}

void FileDropApplication::tooLarge(WFileDropWidget::File *file)
{
  icons_[file]->addStyleClass("invalid");
  
  log_->setText("File too large: " + file->clientFileName());
}

void FileDropApplication::failed(WFileDropWidget::File *file)
{
  icons_[file]->addStyleClass("invalid");
  
  log_->setText("Upload failed: " + file->clientFileName());
}

void FileDropApplication::saveFile(WFileDropWidget::File *file)
{
  std::string spool = file->uploadedFile().spoolFileName();
  std::ifstream src(spool.c_str(), std::ios::binary);

  bool isFiltered = ends_with(file->clientFileName(), ".fastq");
  std::string saveName = isFiltered ?
    (UPLOAD_FOLDER + file->clientFileName() + ".gz") :
    (UPLOAD_FOLDER + file->clientFileName());
  
  std::ofstream dest(saveName.c_str(), std::ios::binary);
  if (dest.fail()) {
    std::cerr << "**** ERROR: The output file could not be opened"
  	      << std::endl;
    return;
  }
  
  dest << src.rdbuf();

  if (icons_.find(file) != icons_.end()) {
    icons_[file]->addStyleClass("ready");
    drop_->remove(file);
  }
}

void FileDropApplication::updateProgressListener()
{
  // if there is a next file listen for progress
  if (drop_->currentIndex() < drop_->uploads().size()) {
    WFileDropWidget::File *file = drop_->uploads()[drop_->currentIndex()];
    file->dataReceived().connect(this, &FileDropApplication::showProgress);
    std::string fileName = Utils::htmlEncode(file->clientFileName());
    log_->setText("uploading file &quot;" + fileName + "&quot;");
  }
}

void FileDropApplication::showProgress(::uint64_t current, ::uint64_t total)
{
  WFileDropWidget::File *file = drop_->uploads()[drop_->currentIndex()];
  std::string c = boost::lexical_cast<std::string>(current/1024);
  std::string t = boost::lexical_cast<std::string>(total/1024);
  std::string fileName = Utils::htmlEncode(file->clientFileName());
  log_->setText("uploading file <i>&quot;" + fileName + "&quot;</i>"
		+ " (" + c + "kB" + " out of " + t + "kB)");
}
