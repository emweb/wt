/*
 * Copyright (C) 2016 Emweb bvba, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "FileDropApplication.h"

#include <Wt/WContainerWidget>
#include <Wt/WPushButton>
#include <Wt/WText>
#include <Wt/WFileDropWidget>
#include "Wt/Utils"

#include <iostream>
#include <fstream>

using namespace Wt;

static const std::string UPLOAD_FOLDER = "./uploaded/";
static const int MAX_FILES = 36;


FileDropApplication::FileDropApplication(const WEnvironment& env)
  : WApplication(env),
    nbUploads_(0)
{
  setTitle("File Drop Example");
  useStyleSheet("style.css");

  new WText("<h1>Try dropping a file in the widget below</h1>", root());

  drop_ = new WFileDropWidget(root());

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
}

void FileDropApplication::handleDrop(std::vector<WFileDropWidget::File *> files)
{
  for (unsigned i=0; i < files.size(); i++) {
    if (nbUploads_ >= MAX_FILES) {
      drop_->cancelUpload(files[i]);
      continue;
    }

    WContainerWidget *block = new WContainerWidget(drop_);
    block->setToolTip(files[i]->clientFileName() + " [" + files[i]->mimeType()
		      + "]");
    block->addStyleClass("upload-block");

    icons_[files[i]] = block;
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
  // std::string spool = file->uploadedFile().spoolFileName();
  // std::ifstream src(spool.c_str(), std::ios::binary);

  // std::ofstream dest((UPLOAD_FOLDER + file->clientFileName()).c_str(),
  // 		       std::ios::binary);
  // if (dest.fail()) {
  //   std::cerr << "**** ERROR: The output file could not be opened"
  // 	      << std::endl;
  //   return;;
  // }
  
  // dest << src.rdbuf();

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
