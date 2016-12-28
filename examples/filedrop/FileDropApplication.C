/*
 * Copyright (C) 2016 Emweb bvba, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "FileDropApplication.h"

#include <Wt/WContainerWidget.h>
#include <Wt/WPushButton.h>
#include <Wt/WText.h>
#include <Wt/WFileDropWidget.h>
#include "Wt/Utils.h"

#include <iostream>
#include <fstream>

static const std::string UPLOAD_FOLDER = "./uploaded/";
static const int MAX_FILES = 36;


FileDropApplication::FileDropApplication(const Wt::WEnvironment& env)
  : WApplication(env),
    nbUploads_(0)
{
  setTitle("File Drop Example");
  useStyleSheet("style.css");

  root()->addWidget(Wt::cpp14::make_unique<Wt::WText>("<h1>Try dropping a file in the widget below</h1>"));

  drop_ = root()->addWidget(Wt::cpp14::make_unique<Wt::WFileDropWidget>());

  drop_->drop().connect(this, &FileDropApplication::handleDrop);
  drop_->newUpload().connect(this,&FileDropApplication::updateProgressListener);
  drop_->uploaded().connect(this, &FileDropApplication::saveFile);
  drop_->uploadFailed().connect(this, &FileDropApplication::failed);
  drop_->tooLarge().connect(this, &FileDropApplication::tooLarge);
  
  log_ = root()->addWidget(Wt::cpp14::make_unique<Wt::WText>());
  log_->setInline(false);
  log_->setTextFormat(Wt::TextFormat::XHTML);

  Wt::WPushButton *abort = root()->addWidget(
        Wt::cpp14::make_unique<Wt::WPushButton>("Abort current upload"));
  abort->clicked().connect(this, &FileDropApplication::cancelUpload);
}

void FileDropApplication::handleDrop(std::vector<Wt::WFileDropWidget::File *> files)
{
  for (unsigned i=0; i < files.size(); i++) {
    if (nbUploads_ >= MAX_FILES) {
      drop_->cancelUpload(files[i]);
      continue;
    }

    Wt::WContainerWidget *block =
        drop_->addWidget(Wt::cpp14::make_unique<Wt::WContainerWidget>());
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
  
  Wt::WFileDropWidget::File *currentFile = drop_->uploads()[drop_->currentIndex()];
  drop_->cancelUpload(currentFile);
  icons_[currentFile]->addStyleClass("cancelled");
}

void FileDropApplication::tooLarge(Wt::WFileDropWidget::File *file, ::uint64_t)
{
  icons_[file]->addStyleClass("invalid");
  
  log_->setText("File too large: " + file->clientFileName());
}

void FileDropApplication::failed(Wt::WFileDropWidget::File *file)
{
  icons_[file]->addStyleClass("invalid");
  
  log_->setText("Upload failed: " + file->clientFileName());
}

void FileDropApplication::saveFile(Wt::WFileDropWidget::File *file)
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
    Wt::WFileDropWidget::File *file = drop_->uploads()[drop_->currentIndex()];
    file->dataReceived().connect(this, &FileDropApplication::showProgress);
    std::string fileName = Wt::Utils::htmlEncode(file->clientFileName());
    log_->setText("uploading file &quot;" + fileName + "&quot;");
  }
}

void FileDropApplication::showProgress(::uint64_t current, ::uint64_t total)
{
  Wt::WFileDropWidget::File *file = drop_->uploads()[drop_->currentIndex()];
  std::string c = std::to_string(current/1024);
  std::string t = std::to_string(total/1024);
  std::string fileName = Wt::Utils::htmlEncode(file->clientFileName());
  log_->setText("uploading file <i>&quot;" + fileName + "&quot;</i>"
		+ " (" + c + "kB" + " out of " + t + "kB)");
}
