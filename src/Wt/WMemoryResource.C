/*
 * Copyright (C) 2007 Wim Dumon, Leuven, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WMemoryResource"

namespace Wt {

WMemoryResource::WMemoryResource(WObject *parent)
  : WResource(parent)
{
  setReentrant(true);
}

WMemoryResource::WMemoryResource(const std::string& mimeType,
				 WObject *parent)
  : WResource(parent),
    mimeType_(mimeType)
{
  setReentrant(true);
}

WMemoryResource::WMemoryResource(const std::string& mimeType,
				 const std::vector<char> &data,
				 WObject *parent)
  : WResource(parent),
    mimeType_(mimeType),
    data_(data)
{
  setReentrant(true);
}

void WMemoryResource::setMimeType(const std::string& mimeType)
{
  mimeType_ = mimeType;
  dataChanged.emit();
}

void WMemoryResource::setData(const std::vector<char>& data)
{
  data_ = data;
  dataChanged.emit();
}

void WMemoryResource::setData(const char *data, int count)
{
  data_.clear();
  data_.insert(data_.end(), data, data + count);

  dataChanged.emit();
}

const std::string WMemoryResource::resourceMimeType() const
{
  return mimeType_;
}

bool WMemoryResource::streamResourceData(std::ostream& stream,
					 const ArgumentMap& arguments)
{
  for(unsigned int i = 0; i < data_.size(); ++i) {
    stream.put(data_[i]);
  }

  return true;
}

}
