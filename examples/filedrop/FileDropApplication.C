/*
 * Copyright (C) 2016 Emweb bvba, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "FileDropApplication.h"

#include <Wt/WContainerWidget>
#include <Wt/WText>
#include <Wt/WFileDropWidget>
#include <Wt/WFileUpload>

#include <iostream>
#include <fstream>

using namespace Wt;

static const std::string UPLOAD_FOLDER = "./uploaded/";

FileDropApplication::FileDropApplication(const WEnvironment& env)
  : WApplication(env)
{
  setTitle("File Drop Example");

  new WText("<h1>Try dropping a file in the widget below</h1>", root());
  
  drop_ = new WFileDropWidget();
  root()->addWidget(drop_);

  connectFileUpload();
  
  log_ = new WText(root());
}

void FileDropApplication::connectFileUpload()
{
  drop_->fileUpload()->setMultiple(true);
  drop_->fileUpload()->uploaded().connect(this, &FileDropApplication::saveFile);
  drop_->fileUpload()->fileTooLarge().connect(this,
					      &FileDropApplication::fileTooBig);
}

void FileDropApplication::saveFile()
{
  std::vector<Http::UploadedFile> files = drop_->fileUpload()->uploadedFiles();
  for (int i =0; i < files.size(); i++) {
    std::string spool = files[i].spoolFileName();
    
    std::ifstream src(spool.c_str(), std::ios::binary);
    std::ofstream dest((UPLOAD_FOLDER + files[i].clientFileName()).c_str(),
		       std::ios::binary);
    if (dest.fail()) {
      std::cerr << "**** ERROR: The output file could not be opened"
		<< std::endl;
      break;
    }
    
    dest << src.rdbuf();

    // Add a little block to the container-widget
    WContainerWidget *block = new WContainerWidget(drop_);
    block->setWidth(20);
    block->setHeight(20);
    block->setMargin(10);
    block->decorationStyle().setBackgroundColor(WColor("black"));
  }

  
  log_->setText("Thanks for the file(s)");
  
  drop_->resetUpload();
  connectFileUpload();
}

void FileDropApplication::fileTooBig()
{
  log_->setText("The file is too big ...");
}
