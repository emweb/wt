/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WAbstractServer"

namespace Wt {

WServer *WAbstractServer::instance_ = 0;

WAbstractServer::~WAbstractServer()
{ }

}
