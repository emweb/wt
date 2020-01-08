/*
 * Copyright (C) 2016 Emweb bv, Herent, Belgium.
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
  void addObserver(observing_ptr_base *ptr) noexcept
  {
    observers_.push_back(ptr);
  }

  void removeObserver(observing_ptr_base *ptr) noexcept
  {
    auto i = std::find(observers_.begin(), observers_.end(), ptr);

    if (i != observers_.end())
      observers_.erase(i);
  }

  void replaceObserver(observing_ptr_base *original,
                       observing_ptr_base *observer) noexcept
  {
    auto i = std::find(observers_.begin(), observers_.end(), original);

    if (i != observers_.end())
      *i = observer;
    else
      observers_.push_back(observer);
  }

  ~observer_info() {
    for (auto& o : observers_)
      o->clear();
  }

private:
  std::vector<observing_ptr_base *> observers_;
};

}

observable::observable() noexcept
{ }

observable::~observable()
{ }

void observable::addObserver(Impl::observing_ptr_base *observer) noexcept
{
  if (!observerInfo_)
    observerInfo_.reset(new Impl::observer_info());

  observerInfo_->addObserver(observer);
}

void observable::removeObserver(Impl::observing_ptr_base *observer) noexcept
{
  if (observerInfo_)
    observerInfo_->removeObserver(observer);
}

void observable::replaceObserver(Impl::observing_ptr_base *original,
                                 Impl::observing_ptr_base *observer) noexcept
{
  if (!observerInfo_)
    observerInfo_.reset(new Impl::observer_info());

  observerInfo_->replaceObserver(original, observer);
}

} }
