/*
 * Copyright (C) 2009 Emweb bv, Herent, Belgium
 *
 * See the LICENSE file for terms of use.
 */

#include <iostream>
#include <stdlib.h>
#include <algorithm>

#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WEnvironment.h>
#include <Wt/WLineEdit.h>
#include <Wt/WGridLayout.h>
#include <Wt/WHBoxLayout.h>
#include <Wt/WPushButton.h>
#include <Wt/WTable.h>
#include <Wt/WText.h>
#include <Wt/WTreeView.h>
#include <Wt/WVBoxLayout.h>
#include <Wt/WViewWidget.h>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/exception.hpp>
#include <boost/filesystem/convenience.hpp>
#include <boost/algorithm/string.hpp>

#include "ExampleSourceViewer.h"
#include "FileItem.h"

namespace fs = boost::filesystem;

// Same as p.filename() in latest boost::filesystem
static std::string filename(const fs::path& p)
{
#if BOOST_FILESYSTEM_VERSION < 3
  return p.empty() ? std::string() : *--p.end();
#else
  return p.empty() ? std::string() : (*--p.end()).string();
#endif
}

// Same as p.stem() in latest boost::filesystem
static std::string stem(const fs::path& p)
{
  std::string fn = filename(p);
  std::size_t pos = fn.find('.');
  if (pos == std::string::npos)
    return fn;
  else
    return fn.substr(0, pos);
}

// Should be same as p.parent_path() in latest boost::filesystem
// This is not entirely according to fs::path::parent_path() in 1.39.0
fs::path parent_path(const fs::path& p)
{
  std::string fn = filename(p);
  std::string path = p.string();

  return path.substr(0, path.length() - fn.length() - 1);
}

static bool comparePaths(const fs::path& p1, const fs::path& p2)
{
  return filename(p1) > filename(p2);
}

ExampleSourceViewer::ExampleSourceViewer(const std::string& deployPath,
					 const std::string& examplesRoot,
					 const std::string& examplesType)
  : deployPath_(deployPath),
    examplesRoot_(examplesRoot),
    examplesType_(examplesType)
{
  wApp->internalPathChanged().connect
    (this, &ExampleSourceViewer::handlePathChange);

  handlePathChange();
}

void ExampleSourceViewer::handlePathChange()
{
  WApplication *app = wApp;

  if (app->internalPathMatches(deployPath_)) {
    std::string example = app->internalPathNextPart(deployPath_);

    if (example.find("..") != std::string::npos
	|| example.find('/') != std::string::npos
	|| example.find('\\') != std::string::npos) {
      app->setInternalPathValid(false);
      setExample("INVALID_DIR", "INVALID");
    } else
      setExample(examplesRoot_ + example, example);
  }
}

void ExampleSourceViewer::setExample(const std::string& exampleDir,
				     const std::string& example)
{
  clear();

  bool exists = false;
  try {
    exists = fs::exists(exampleDir);
  } catch (std::exception&) {
  }

  if (!exists) {
    WApplication::instance()->setInternalPathValid(false);
    addWidget(cpp14::make_unique<WText>("No such example: " + exampleDir));
    return;
  }

  model_ = std::make_shared<WStandardItemModel>(0, 1);
  if (examplesType_ == "CPP") {
    cppTraverseDir(model_->invisibleRootItem(), exampleDir);
  } else if (examplesType_ == "JAVA") {
    javaTraverseDir(model_->invisibleRootItem(), exampleDir);
  }

  WApplication::instance()->setTitle(tr("srcview.title." + example));
  std::unique_ptr<WText> title(cpp14::make_unique<WText>(
                                 tr("srcview.title." + examplesType_ + "." + example)));
  title->setInternalPathEncoding(true);

  auto exampleView = cpp14::make_unique<WTreeView>();
  exampleView_ = exampleView.get();
  exampleView_->setHeaderHeight(0);
  exampleView_->resize(300, WLength::Auto);
  exampleView_->setSortingEnabled(false);
  exampleView_->setModel(model_);
  exampleView_->expandToDepth(1);
  exampleView_->setSelectionMode(SelectionMode::Single);
  exampleView_->setAlternatingRowColors(false);
  exampleView_->selectionChanged().connect
    (this, &ExampleSourceViewer::showFile);

  auto sourceView =
        cpp14::make_unique<SourceView>(FileItem::FileNameRole,
                                       FileItem::ContentsRole,
                                       FileItem::FilePathRole);
  sourceView_ = sourceView.get();
  sourceView_->setStyleClass("source-view");

  /*
   * Expand path to first file, to show something in the source viewer
   */
  WStandardItem *w = model_->item(0);
  do {
    exampleView_->setExpanded(w->index(), true);
    if (w->rowCount() > 0)
      w = w->child(0);
    else {
      exampleView_->select(w->index());
      w = 0;
    }
  } while (w);

  auto topLayout = cpp14::make_unique<WVBoxLayout>();
  topLayout->addWidget(std::move(title));

  auto gitLayout = cpp14::make_unique<WHBoxLayout>();
  WHBoxLayout *g = gitLayout.get();
  gitLayout->addWidget(std::move(exampleView), 0);
  gitLayout->addWidget(std::move(sourceView), 1);
  topLayout->addLayout(std::move(gitLayout), 1);
  g->setResizable(0);

  /*
   * FIXME, in plain HTML mode, we should set a minimum size to the source
   * view, and remove this in enableAjax() ?
   */
  // sourceView_->setHeight("100%");

  setLayout(std::move(topLayout));
  setStyleClass("maindiv");
}

/*
 * Return the companion implementation/header file for a C++ source file.
 */
static fs::path getCompanion(const fs::path& path) 
{
  std::string ext = fs::extension(path);

  if (ext == ".h")
    return parent_path(path) / (stem(path) + ".C");
  else if (ext == ".C" || ext == ".cpp")
    return parent_path(path) / (stem(path) + ".h");
  else
    return fs::path();
}

void ExampleSourceViewer::cppTraverseDir(WStandardItem* parent, 
					 const fs::path& path)
{
  static const char *supportedFiles[] = {
    ".C", ".cpp", ".h", ".css", ".xml", ".png", ".gif", ".csv", ".ico", 0
  };

  auto dir = cpp14::make_unique<FileItem>("/icons/yellow-folder-open.png",
                                                             filename(path),
                                                             "");
  FileItem *dirPtr = dir.get();
  parent->appendRow(std::move(dir));
  parent = dirPtr;
  try {
    std::set<fs::path> paths;

    fs::directory_iterator end_itr;
    for (fs::directory_iterator i(path); i != end_itr; ++i) 
      paths.insert(*i);

    std::vector<std::unique_ptr<FileItem>> classes, files;
    std::vector<fs::path> dirs;

    while (!paths.empty()) {
      fs::path p = *paths.begin();
      paths.erase(p);

      // skip symbolic links and other files
      if (fs::is_symlink(p))
	continue;

      // skip files with an extension we do not want to handle
      if (fs::is_regular(p)) {
	std::string ext = fs::extension(p);
	bool supported = false;
	for (const char **s = supportedFiles; *s != 0; ++s)
	  if (*s == ext) {
	    supported = true;
	    break;
	  }
	
	if (!supported)
	  continue;
      }

      // see if we have one file of a class (.C, .h)
      fs::path companion = getCompanion(p);
      if (!companion.empty()) {
	std::set<fs::path>::iterator it_companion = paths.find(companion);
 
	  if (it_companion != paths.end()) {
	    std::string className = stem(p);
	    escapeText(className);
	    std::string label = "<i>class</i> " + className;

	    std::unique_ptr<FileItem> classItem =
	      cpp14::make_unique<FileItem>("/icons/cppclass.png", label, std::string());
	    classItem->setFlags(classItem->flags() | ItemFlag::XHTMLText);

	    auto header
		= cpp14::make_unique<FileItem>("/icons/document.png", filename(p),
					    p.string());
	    auto cpp
		= cpp14::make_unique<FileItem>("/icons/document.png",
					 filename(*it_companion),
					 (*it_companion).string());
	    classItem->appendRow(std::move(header));
	    classItem->appendRow(std::move(cpp));
	  
	    classes.push_back(std::move(classItem));
	    paths.erase(it_companion);
	  } else {
	    auto file
		= cpp14::make_unique<FileItem>("/icons/document.png", filename(p),
					  p.string());
	    files.push_back(std::move(file));
	  }
      } else if (fs::is_directory(p)) {
	dirs.push_back(p);
      } else {
        auto file
            = cpp14::make_unique<FileItem>("/icons/document.png", filename(p),
				      p.string());
	files.push_back(std::move(file));
      }
    }

    std::sort(dirs.begin(), dirs.end(), comparePaths);

    for (unsigned int i = 0; i < classes.size(); i++)
      parent->appendRow(std::move(classes[i]));

    for (unsigned int i = 0; i < files.size(); i++)
      parent->appendRow(std::move(files[i]));

    for (unsigned int i = 0; i < dirs.size(); i++)
      cppTraverseDir(parent, dirs[i]);
  } catch (fs::filesystem_error& e) {
    std::cerr << e.what() << std::endl;
  }
}

void ExampleSourceViewer::javaTraversePackages(WStandardItem *parent,
					       const fs::path& srcPath,
					       const std::string packageName)
{
  fs::directory_iterator end_itr;

  FileItem *packageItem = nullptr;
  for (fs::directory_iterator i(srcPath); i != end_itr; ++i) {
    fs::path p = *i;
    if (fs::is_regular(p)) {
      if (!packageItem) {
        auto item = cpp14::make_unique<FileItem>("/icons/package.png", packageName, "");
        packageItem = item.get();
        parent->appendRow(std::move(item));
      }

      auto file
          = cpp14::make_unique<FileItem>("/icons/javaclass.png", filename(p),
				    p.string());
      packageItem->appendRow(std::move(file));
    }
  }

  for (fs::directory_iterator i(srcPath); i != end_itr; ++i) {
    fs::path p = *i;
    if (fs::is_directory(p)) {  
      std::string pn = packageName;
      if (!pn.empty())
	pn += ".";
      pn += filename(p);

      javaTraversePackages(parent, p, pn);
    }
  }
}

void ExampleSourceViewer::javaTraverseDir(WStandardItem* parent, 
					  const fs::path& path)
{
  auto dir
      = cpp14::make_unique<FileItem>("/icons/yellow-folder-open.png",
                                     filename(path),"");
  FileItem *dirPtr = dir.get();
  parent->appendRow(std::move(dir));
  parent = dirPtr;

  std::vector<fs::path> files, dirs;

  fs::directory_iterator end_itr;
  for (fs::directory_iterator i(path); i != end_itr; ++i) {
    fs::path p = *i;
    if (fs::is_directory(p)) {
      if (filename(p) == "src") {
        auto dir
            = cpp14::make_unique<FileItem>("/icons/package-folder-open.png",
				     filename(p), "");
	FileItem *dirPtr = dir.get();
	parent->appendRow(std::move(dir));
	javaTraversePackages(dirPtr, p, "");
      } else
	dirs.push_back(p);
    } else {
      files.push_back(p);
    }
  }

  std::sort(dirs.begin(), dirs.end(), comparePaths);
  std::sort(files.begin(), files.end(), comparePaths);

  for (auto item : dirs)
    javaTraverseDir(parent, item);

  for (auto item : files) {
    auto file
        = cpp14::make_unique<FileItem>("/icons/document.png", filename(item),
                                  item.string());
    parent->appendRow(std::move(file));
  }
}

/*! \brief Displayed the currently selected file.
 */
void ExampleSourceViewer::showFile() {
  if (exampleView_->selectedIndexes().empty())
    return;

  WModelIndex selected = *exampleView_->selectedIndexes().begin();

  // expand a folder when clicked
  if (exampleView_->model()->rowCount(selected) > 0
      && !exampleView_->isExpanded(selected))
    exampleView_->setExpanded(selected, true);

  // (for a file,) load data in source viewer
  sourceView_->setIndex(selected);
}
