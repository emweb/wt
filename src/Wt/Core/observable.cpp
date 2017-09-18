/*
 * Copyright (C) 2016 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "observable.hpp"
#include "observing_ptr.hpp"

#include <algorithm>

namespace Wt { namespace Core {

namespace Impl {

struct observer_info
{
  void addObserver(observing_ptr_base *ptr)
  {
    observers_.push_back(ptr);
  }

  void removeObserver(observing_ptr_base *ptr)
  {
    auto i = std::find(observers_.begin(), observers_.end(), ptr);

    if (i != observers_.end())
      observers_.erase(i);
  }

  ~observer_info() {
    for (auto& o : observers_)
      o->clear();
  }

private:
  std::vector<observing_ptr_base *> observers_;
};

}

observable::observable()
{ }

observable::~observable()
{ }

void observable::addObserver(Impl::observing_ptr_base *observer)
{
  if (!observerInfo_)
    observerInfo_.reset(new Impl::observer_info());

  observerInfo_->addObserver(observer);
}

void observable::removeObserver(Impl::observing_ptr_base *observer)
{
  if (observerInfo_)
    observerInfo_->removeObserver(observer);
}

} }
