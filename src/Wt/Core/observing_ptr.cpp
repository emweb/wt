/*
 * Copyright (C) 2016 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "observing_ptr.hpp"

namespace Wt { namespace Core {

namespace Impl {

observing_ptr_base::observing_ptr_base()
  : observed_(nullptr),
    cleared_(false)
{ }

observing_ptr_base::~observing_ptr_base()
{ 
  if (observed_)
    observed_->removeObserver(this);
}

void observing_ptr_base::clear()
{
  observed_ = nullptr;
  cleared_ = true;
}

void observing_ptr_base::set(observable *observable)
{
  cleared_ = false;
  observed_ = observable;
  observed_->addObserver(this);
}

}

} }
