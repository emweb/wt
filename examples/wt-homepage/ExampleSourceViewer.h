// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2009 Emweb bvba 
 *
 * See the LICENSE file for terms of use.
 */

#include <iostream>
#include <stdlib.h>

#include <Wt/WContainerWidget>
#include <Wt/WTreeView>
#include <Wt/WStandardItemModel>

#include "FileItem.h"
#include "SourceView.h"

/*! \class ExampleSourceViewer 
 *  \brief A simple widget to visualise a set of example source files.
 */
class ExampleSourceViewer: public Wt::WContainerWidget
{
public:
  /*! \brief Constructor.
   */
  ExampleSourceViewer(const std::string& deployPath,
		      const std::string& examplesRoot,
		      const std::string& examplesType); 

private:
  Wt::WTreeView  *exampleView_;
  SourceView *sourceView_;

  std::string deployPath_;
  std::string examplesRoot_;
  std::string examplesType_;

  Wt::WStandardItemModel *model_;

  void cppTraverseDir(Wt::WStandardItem* parent, 
		      const boost::filesystem::path& path);
  void javaTraverseDir(Wt::WStandardItem* parent, 
		       const boost::filesystem::path& path);
  void javaTraversePackages(Wt::WStandardItem *parent,
			    const boost::filesystem::path& srcPath,
			    const std::string packageName);

  /*! \brief Displayed the currently selected file.
   */
  void showFile(); 

  void handlePathChange();

  void setExample(const std::string& exampleDir,
		  const std::string& example);
};
