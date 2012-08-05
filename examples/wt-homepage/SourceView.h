// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2009 Emweb bvba 
 *
 * See the LICENSE file for terms of use.
 */

#ifndef SOURCEVIEW_H
#define SOURCEVIEW_H

#include <Wt/WViewWidget>
#include <Wt/WModelIndex>
#include <Wt/WMemoryResource>

/**
 * \defgroup gitmodelexample Git model example
 */
/*@{*/

/*! \class SourceView
 *  \brief View class for source code.
 *
 * A view class is used so that no server-side memory is used while displaying
 * a potentially large file.
 */
class SourceView : public Wt::WViewWidget
{
public:
  /*! \brief Constructor.
   *
   * The <i>fileNameRole</i> will be used to retrieve data from a file
   * to be displayed. If no data is set for this role, then
   * <i>contentRole</i> should hold the data as a string.
   */
  SourceView(int fileNameRole, int contentRole, int filePathRole);
 
  /*! \brief Destructor
   */
  virtual ~SourceView();

  /*! \brief Sets the model index.
   *
   * Returns true whether the view will be rerendered. The view will only
   * be rerendered if the index contains new data.
   */
  bool setIndex(const Wt::WModelIndex& index); 

  /*! \brief Returns the widget that renders the view.
   *
   * Returns he view contents: renders the file to a WText widget.
   * WViewWidget deletes this widget after every rendering step.
   */
  virtual Wt::WWidget *renderView(); 
  
private:
  /// The index that is currently displayed.
  Wt::WModelIndex index_;

  /// The role that is currently displayed.
  int fileNameRole_;
  int contentRole_;
  int filePathRole_;

  Wt::WMemoryResource* imageResource_;

private: 
  std::string imageExtension(const std::string& fileName);
};

/*@}*/

#endif //SOURCEVIEW_H
