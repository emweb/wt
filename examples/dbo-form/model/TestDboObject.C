/*
 * Copyright (C) 2021 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "TestDboObject.h"

#include <Wt/Dbo/Impl.h>

DBO_INSTANTIATE_TEMPLATES(TestDboPtr)
DBO_INSTANTIATE_TEMPLATES(TestDboObject)

TestDboPtr::TestDboPtr()
{}

TestDboPtr::TestDboPtr(const Wt::WString& name)
  : name_(name)
{
}

TestDboObject::TestDboObject()
{}

