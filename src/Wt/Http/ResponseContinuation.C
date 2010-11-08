/*
 * Copyright (C) 2009 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <iostream>

#include "Wt/Http/ResponseContinuation"
#include "Wt/WResource"

#include "WebRequest.h"

namespace Wt {
  namespace Http {

void ResponseContinuation::setData(const boost::any& data)
{
  data_ = data;
}

void ResponseContinuation::doContinue()
{
  // We are certain that the continuation is still "alive" because it is
  // protected by a mutex, and thus a simultaneous change with
  // WebResponse::flush() is not possible: ResponseContinuation::stop(),
  // called before destruction together with the resource, will thus
  // block while we are here.
  resource_->doContinue(this);
}

ResponseContinuation::ResponseContinuation(WResource *resource,
					   WebResponse *response)
  : resource_(resource),
    response_(response)
{
  resource_->continuations_.push_back(this);
}

void ResponseContinuation::stop()
{
  response_->flush(WebResponse::ResponseDone);
}

ResponseContinuation::~ResponseContinuation()
{ }

  }
}
