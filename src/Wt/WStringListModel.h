// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WSTRING_LIST_MODEL_H_
#define WSTRING_LIST_MODEL_H_

#include <Wt/WAbstractListModel.h>

namespace Wt {

/*! \class WStringListModel Wt/WStringListModel.h Wt/WStringListModel.h
 *  \brief An model that manages a list of strings.
 *
 * This model only manages a unidimensional list of items and is
 * optimized for usage by view widgets such as combo-boxes.
 *
 * It supports all features of a typical item model, including data
 * for multiple roles, editing and addition and removal of data rows.
 *
 * You can populate the model by passing a list of strings to its
 * consructor, or by using the setStringList() method. You can set or
 * retrieve data using the setData() and data() methods, and add or
 * remove data using the insertRows() and removeRows() methods.
 *
 * \sa WComboBox, WSelectionBox
 *
 * \ingroup modelview
 */
class WT_API WStringListModel : public WAbstractListModel
{
public:
  /*! \brief Creates a new empty string list model.
   */
  WStringListModel();

  /*! \brief Creates a new string list model.
   */
  WStringListModel(const std::vector<WString>& strings);

  /*! \brief Destructor.
   */
  ~WStringListModel();

  /*! \brief Sets a new string list.
   *
   * Replaces the current string list with a new list.
   *
   * \sa dataChanged()
   * \sa addString()
   */
  void setStringList(const std::vector<WString>& strings);

  /*! \brief Inserts a string.
   *
   * \sa setStringList()
   */
  void insertString(int row, const WString& string);

  /*! \brief Adds a string.
   *
   * \sa setStringList()
   */
  void addString(const WString& string);

  /*! \brief Returns the string list.
   *
   * \sa setStringList()
   */
  const std::vector<WString>& stringList() const { return displayData_; }

  /*! \brief Sets model flags for an item.
   *
   * The default item flags are \link Wt::ItemFlag::Selectable
   * ItemFlag::Selectable\endlink | \link Wt::ItemFlag::Editable
   * ItemFlag::Editable\endlink.
   */
  void setFlags(int index, WFlags<ItemFlag> flags);

  /*! \brief Returns the flags for an item.
   *
   * This method is reimplemented to return flags set in setFlags().
   *
   * \sa setFlags()
   */
  virtual WFlags<ItemFlag> flags(const WModelIndex& index) const override;

  using WAbstractListModel::setData;
  virtual bool setData(const WModelIndex& index, const cpp17::any& value,
                       ItemDataRole role = ItemDataRole::Edit) override;

  virtual cpp17::any data(const WModelIndex& index, ItemDataRole role = ItemDataRole::Display)
    const override;

  virtual int rowCount(const WModelIndex& parent = WModelIndex())
    const override;

  virtual bool insertRows(int row, int count,
			  const WModelIndex& parent = WModelIndex()) override;

  virtual bool removeRows(int row, int count,
			  const WModelIndex& parent = WModelIndex()) override;

  virtual void sort(int column,
		    SortOrder order = SortOrder::Ascending) override;

private:
#ifndef WT_TARGET_JAVA
  typedef std::map<ItemDataRole, cpp17::any> DataMap;
#else
  typedef std::treemap<ItemDataRole, cpp17::any> DataMap;
#endif

  std::vector<WString> displayData_;
  std::vector<DataMap> *otherData_;
  std::vector<WFlags<ItemFlag> > flags_;
};

}

#endif // WSTRING_LIST_MODEL_H_
