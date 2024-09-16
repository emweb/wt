/*
 * Copyright (C) 2016 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "FileDropApplication.h"

#include <Wt/WContainerWidget.h>
#include <Wt/WPushButton.h>
#include <Wt/WTemplate.h>
#include <Wt/WText.h>
#include <Wt/WFileDropWidget.h>
#include "Wt/Utils.h"

#include <Wt/cpp17/filesystem.hpp>

#include <algorithm>
#include <iostream>
#include <fstream>

#ifdef USE_CUSTOM_TRANSFER
#include "custom_transfer/MyFileDropWidget.h"
#endif

using namespace Wt;

static const std::string UPLOAD_FOLDER = "./uploaded/";
static const int MAX_DROPS = 25;

FileDropApplication::FileDropApplication(const Wt::WEnvironment& env)
  : WApplication(env)
{
  setTitle("File Drop Example");

  useStyleSheet("style.css");
  useStyleSheet("resources/font-awesome/css/font-awesome.min.css");
  messageResourceBundle().use("templates");

  tpl_ = root()->addNew<WTemplate>(WTemplate::tr("FileDropApplication.template"));

#ifdef USE_CUSTOM_TRANSFER
  drop_ = tpl_->bindNew<MyFileDropWidget>("dropwidget");
#else
  drop_ = tpl_->bindNew<WFileDropWidget>("dropwidget");
#endif

  drop_->setDropIndicationEnabled(true);
  // drop_->setGlobalDropEnabled(true);
  drop_->setAcceptDirectories(true);

  drop_->drop().connect(this, &FileDropApplication::handleDrop);
  drop_->newUpload().connect(this,&FileDropApplication::updateProgressListener);
  drop_->uploaded().connect(this, &FileDropApplication::saveFile);
  drop_->uploadFailed().connect(this, &FileDropApplication::failed);
  drop_->tooLarge().connect(this, &FileDropApplication::tooLarge);

  auto selectFileBtn = tpl_->bindNew<WPushButton>("select-file-btn", "Select File");
  selectFileBtn->clicked().connect(drop_, &WFileDropWidget::openFilePicker);
  auto selectDirBtn = tpl_->bindNew<WPushButton>("select-dir-btn", "Select Directory");
  selectDirBtn->clicked().connect(drop_, &WFileDropWidget::openDirectoryPicker);

  progress_ = tpl_->bindNew<WText>("progress-msg");
  auto abort = tpl_->bindNew<WPushButton>("abort-btn", "abort");
  abort->clicked().connect(this, &FileDropApplication::cancelUpload);
  tpl_->setCondition("if:progress", false);

  log_ = tpl_->bindNew<WContainerWidget>("log");
}

void FileDropApplication::handleDrop(std::vector<Wt::WFileDropWidget::File *> files)
{
  for (unsigned i=0; i < files.size(); i++) {
    WFileDropWidget::File *file = files[i];
    if (icons_.size() >= MAX_DROPS) {
      drop_->cancelUpload(file);
      continue;
    }

    auto block = drop_->addNew<Wt::WContainerWidget>();
    block->setToolTip(file->path());
    block->addStyleClass("upload-block");

    icons_[file] = block;
    if (file->directory()) {
      block->addStyleClass("fa fa-folder");
      auto dir = dynamic_cast<WFileDropWidget::Directory*>(file);
      for (auto *dirFile : dir->contents()) {
        dirFiles_[dirFile] = dir;
      }
    } else {
      block->addStyleClass("fa fa-file");
    }

    addLogLine("New item dropped: " + file->path());
  }

  if (icons_.size() >= MAX_DROPS) {
    drop_->setAcceptDrops(false);
    addLogLine("Maximum number of item drops reached, disabling widget.");
  }
}

void FileDropApplication::cancelUpload()
{
  if (drop_->uploads().size() == static_cast<size_t>(drop_->currentIndex()))
    return;

  auto *currentFile = drop_->uploads()[drop_->currentIndex()];
  drop_->cancelUpload(currentFile);
  addLogLine("Upload cancelled: " + currentFile->path());
  tpl_->setCondition("if:progress", false);

  getIcon(currentFile)->addStyleClass("cancelled");
}

void FileDropApplication::tooLarge(Wt::WFileDropWidget::File *file, ::uint64_t)
{
  getIcon(file)->addStyleClass("invalid");

  addLogLine("File too large: " + file->clientFileName());
  tpl_->setCondition("if:progress", false);
}

void FileDropApplication::failed(Wt::WFileDropWidget::File *file)
{
  getIcon(file)->addStyleClass("invalid");

  addLogLine("Upload failed: " + file->clientFileName());
  tpl_->setCondition("if:progress", false);
}

void FileDropApplication::saveFile(Wt::WFileDropWidget::File *file)
{
  std::string spool = file->uploadedFile().spoolFileName();
  std::ifstream src(spool.c_str(), std::ios::binary);

  auto saveName = Wt::cpp17::filesystem::path(appRoot() + UPLOAD_FOLDER) / file->path();
  Wt::cpp17::filesystem::create_directories(saveName.parent_path());

  std::ofstream dest(saveName.c_str(), std::ios::binary);
  if (dest.fail()) {
    std::cerr << "**** ERROR: The output file could not be opened: " << saveName
                << std::endl;
    return;
  }

  dest << src.rdbuf();

  if (dirFiles_.find(file) != dirFiles_.end()) {
    WFileDropWidget::Directory *dir = dirFiles_[file];
    const std::vector<WFileDropWidget::File*>& contents = dir->contents();
    bool isReady = std::all_of(contents.begin(), contents.end(), [] (auto *f) {
        return f->uploadFinished();
      });
    if (isReady) {
      icons_[dir]->addStyleClass("ready");
    }
  } else {
    icons_[file]->addStyleClass("ready");
  }

  addLogLine("Upload finished: " + file->path());
  tpl_->setCondition("if:progress", false);
}

void FileDropApplication::updateProgressListener()
{
  // if there is a next file listen for progress
  if (static_cast<size_t>(drop_->currentIndex()) < drop_->uploads().size()) {
    Wt::WFileDropWidget::File *file = drop_->uploads()[drop_->currentIndex()];
    file->dataReceived().connect(this, &FileDropApplication::showProgress);
    std::string fileName = Wt::Utils::htmlEncode(file->clientFileName());
    progress_->setText("uploading file &quot;" + fileName + "&quot;");
    tpl_->setCondition("if:progress", true);
  }
}

void FileDropApplication::showProgress(::uint64_t current, ::uint64_t total)
{
  Wt::WFileDropWidget::File *file = drop_->uploads()[drop_->currentIndex()];
  std::string c = std::to_string(current/1024);
  std::string t = std::to_string(total/1024);
  std::string fileName = Wt::Utils::htmlEncode(file->clientFileName());
  progress_->setText("uploading file <i>&quot;" + fileName + "&quot;</i>"
                + " (" + c + "kB" + " out of " + t + "kB)");
}

Wt::WContainerWidget* FileDropApplication::getIcon(Wt::WFileDropWidget::File *file)
{
  if (dirFiles_.find(file) != dirFiles_.end()) {
    Wt::WFileDropWidget::File *dir = dirFiles_[file];
    return icons_[dir];
  } else {
    return icons_[file];
  }
}

void FileDropApplication::addLogLine(const std::string& msg)
{
  log_->addNew<WText>("<div>" + msg + "</div>");
}
