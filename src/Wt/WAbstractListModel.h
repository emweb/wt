// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WABSTRACT_LIST_MODEL_H_
#define WABSTRACT_LIST_MODEL_H_

#include <Wt/WAbstractItemModel.h>

namespace Wt {

/*! \class WAbstractListModel Wt/WAbstractListModel.h Wt/WAbstractListModel.h
 *  \brief An abstract list model for use with %Wt's view classes.
 *
 * An abstract list model specializes WAbstractItemModel for
 * one-dimensional lists (i.e. a model with 1 column and no children).
 *
 * It cannot be used directly but must be subclassed. Subclassed
 * models must at least reimplement rowCount() to return the number of
 * rows, and data() to return data.
 *
 * \ingroup modelview
 */
class WT_API WAbstractListModel : public WAbstractItemModel
{
public:
  /*! \brief Create a new abstract list model.
   */
  WAbstractListModel();

  /*! \brief Destructor.
   */
  ~WAbstractListModel();

  virtual WModelIndex parent(const WModelIndex& index) const override;
  virtual WModelIndex index(int row, int column = 0,
                            const WModelIndex& parent = WModelIndex()) const override;
  virtual int columnCount(const WModelIndex& parent = WModelIndex()) const override;
};

}

#endif // WABSTRACT_LIST_MODEL_H_
