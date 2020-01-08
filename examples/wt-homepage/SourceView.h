// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2009 Emweb bv, Herent, Belgium
 *
 * See the LICENSE file for terms of use.
 */

#ifndef SOURCEVIEW_H
#define SOURCEVIEW_H

#include <Wt/WViewWidget.h>
#include <Wt/WModelIndex.h>
#include <Wt/WMemoryResource.h>

using namespace Wt;

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
class SourceView : public WViewWidget
{
public:
  /*! \brief Constructor.
   *
   * The <i>fileNameRole</i> will be used to retrieve data from a file
   * to be displayed. If no data is set for this role, then
   * <i>contentRole</i> should hold the data as a string.
   */
  SourceView(ItemDataRole fileNameRole,
             ItemDataRole contentRole,
             ItemDataRole filePathRole);
 
  /*! \brief Destructor
   */
  virtual ~SourceView();

  /*! \brief Sets the model index.
   *
   * Returns true whether the view will be rerendered. The view will only
   * be rerendered if the index contains new data.
   */
  bool setIndex(const WModelIndex& index);

  /*! \brief Returns the widget that renders the view.
   *
   * Returns he view contents: renders the file to a WText widget.
   * WViewWidget deletes this widget after every rendering step.
   */
  virtual std::unique_ptr<WWidget> renderView();
  
private:
  /// The index that is currently displayed.
  WModelIndex index_;

  /// The role that is currently displayed.
  Wt::ItemDataRole fileNameRole_;
  Wt::ItemDataRole contentRole_;
  Wt::ItemDataRole filePathRole_;

  std::shared_ptr<WMemoryResource> imageResource_;

private: 
  std::string imageExtension(const std::string& fileName);
};

/*@}*/

#endif //SOURCEVIEW_H
