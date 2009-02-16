/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <iostream>

#include "Wt/WModelIndex"
#include "Wt/WAbstractItemModel"

namespace Wt {

WModelIndex::WModelIndex()
  : model_(0),
    row_(0),
    column_(0)
{
  memset(internalId_.c_array(), 0, 20);
}

WModelIndex::WModelIndex(const WModelIndex& other)
  : model_(other.model_),
    row_(other.row_),
    column_(other.column_),
    internalId_(other.internalId_)
{ }

boost::any WModelIndex::data(int role) const
{
  return model_->data(*this, role);
}

int WModelIndex::flags() const
{
  return model_->flags(*this);
}

WModelIndex WModelIndex::child(int row, int column) const
{
  return model_ ? model_->index(row, column, *this) : WModelIndex();
}

WModelIndex WModelIndex::parent() const
{
  return model_ ? model_->parent(*this) : WModelIndex();
}

void WModelIndex::getAncestors(std::vector<WModelIndex>& ancestors) const
{
  if (isValid()) {
    parent().getAncestors(ancestors);
    ancestors.push_back(*this);
  }
}

bool WModelIndex::operator== (const WModelIndex& other) const
{
  return (model_ == other.model_
	  && row_ == other.row_
	  && column_ == other.column_
	  && internalId_ == other.internalId_);
}

bool WModelIndex::operator!= (const WModelIndex& other) const
{
  return !(*this == other);
}

bool WModelIndex::operator< (const WModelIndex& i2) const
{
  const WModelIndex& i1 = *this;

  if (!i1.isValid())
    return i2.isValid();
  else if (!i2.isValid())
    return false;
  else if (i1 == i2)
    return false;
  else if (i1.model() != i2.model()) {
    std::cerr << "Comparing indexes from different models are you?"
	      << std::endl;
    return false;
  }

  std::vector<WModelIndex> ancestors1;
  std::vector<WModelIndex> ancestors2;

  i1.getAncestors(ancestors1);
  i2.getAncestors(ancestors2);

  unsigned e = std::min(ancestors1.size(), ancestors2.size());

  for (unsigned i =0; i < e; ++i) {
    WModelIndex a1 = ancestors1[i];
    WModelIndex a2 = ancestors2[i];

    if (a1 != a2) {
      if (a1.row() < a2.row())
	return true;
      else if (a1.row() > a2.row())
	return false;
      else if (a1.column() < a2.column())
	return true;
      else
	return false;
    }
  }

  return ancestors1.size() < ancestors2.size();
}

bool WModelIndex::UnorderedLess::operator() (const WModelIndex& i1,
					     const WModelIndex& i2) const
{
  if (!i1.isValid())
    return i2.isValid();
  else if (!i2.isValid())
    return false;
  else if (i1 == i2)
    return false;
  else if (i1.model() != i2.model()) {
    std::cerr << "Comparing indexes from different models are you?"
	      << std::endl;
    return false;
  } else if (i1.row() < i2.row())
    return true;
  else if (i1.row() > i2.row())
    return false;
  else if (i1.column() < i2.column())
    return true;
  else if (i1.column() > i2.column())
    return false;
  else
    return i1.internalId_ < i2.internalId_;
}

WModelIndex::WModelIndex(int row, int column, const WAbstractItemModel *model,
			 void *ptr)
  : model_(model),
    row_(row),
    column_(column)
{
  memset(internalId_.c_array(), 0, 20);
  *reinterpret_cast<void **>(internalId_.c_array()) = ptr;
}

WModelIndex::WModelIndex(int row, int column, const WAbstractItemModel *model,
			 uint64_t id)
  : model_(model),
    row_(row),
    column_(column)
{
  memset(internalId_.c_array(), 0, 20);
 *reinterpret_cast<uint64_t *>(internalId_.c_array()) = id;
}

WModelIndex::WModelIndex(int row, int column, const WAbstractItemModel *model,
			 const Sha1::Digest& id)
  : model_(model),
    row_(row),
    column_(column),
    internalId_(id)
{ }

}
