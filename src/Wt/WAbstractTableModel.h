// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WABSTRACT_TABLE_MODEL_H_
#define WABSTRACT_TABLE_MODEL_H_

#include <Wt/WAbstractItemModel.h>

namespace Wt {

/*! \class WAbstractTableModel Wt/WAbstractTableModel.h Wt/WAbstractTableModel.h
 *  \brief An abstract table model for use with %Wt's view classes.
 *
 * An abstract table model specializes WAbstractItemModel for
 * two-dimensional tables (but no hierarchical models).
 *
 * It cannot be used directly but must be subclassed. Subclassed
 * models must at least reimplement columnCount(), rowCount() and data().
 *
 * \ingroup modelview
 */
class WT_API WAbstractTableModel : public WAbstractItemModel
{
public:
  /*! \brief Creates a new abstract list model.
   */
  WAbstractTableModel();

  /*! \brief Destructor.
   */
  ~WAbstractTableModel();

  virtual WModelIndex parent(const WModelIndex& index) const override;
  virtual WModelIndex index(int row, int column,
                            const WModelIndex& parent = WModelIndex()) const override;
};

}

#endif // WABSTRACT_TABLE_MODEL_H_
