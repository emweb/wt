// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2009 Emweb bv, Herent, Belgium
 *
 * See the LICENSE file for terms of use.
 */

#include <iostream>
#include <stdlib.h>

#include <Wt/WContainerWidget.h>
#include <Wt/WTreeView.h>
#include <Wt/WStandardItemModel.h>

#include "FileItem.h"
#include "SourceView.h"

using namespace Wt;

/*! \class ExampleSourceViewer 
 *  \brief A simple widget to visualise a set of example source files.
 */
class ExampleSourceViewer: public WContainerWidget
{
public:
  /*! \brief Constructor.
   */
  ExampleSourceViewer(const std::string& deployPath,
		      const std::string& examplesRoot,
		      const std::string& examplesType); 

private:
  WTreeView  *exampleView_;
  SourceView *sourceView_;

  std::string deployPath_;
  std::string examplesRoot_;
  std::string examplesType_;

  std::shared_ptr<WStandardItemModel> model_;

  void cppTraverseDir(WStandardItem* parent,
		      const boost::filesystem::path& path);
  void javaTraverseDir(WStandardItem* parent,
		       const boost::filesystem::path& path);
  void javaTraversePackages(WStandardItem *parent,
			    const boost::filesystem::path& srcPath,
			    const std::string packageName);

  /*! \brief Displayed the currently selected file.
   */
  void showFile(); 

  void handlePathChange();

  void setExample(const std::string& exampleDir,
		  const std::string& example);
};
