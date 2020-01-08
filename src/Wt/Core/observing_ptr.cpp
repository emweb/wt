/*
 * Copyright (C) 2016 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "observing_ptr.hpp"

namespace Wt { namespace Core {

namespace Impl {

observing_ptr_base::observing_ptr_base() noexcept
  : observed_(nullptr),
    cleared_(false)
{ }

observing_ptr_base::~observing_ptr_base()
{ 
  if (observed_)
    observed_->removeObserver(this);
}

void observing_ptr_base::clear() noexcept
{
  observed_ = nullptr;
  cleared_ = true;
}

void observing_ptr_base::set(observable *observable) noexcept
{
  cleared_ = false;
  observed_ = observable;
  observed_->addObserver(this);
}

}

} }
