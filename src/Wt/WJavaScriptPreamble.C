/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WJavaScriptPreamble.h>

namespace Wt {

WJavaScriptPreamble::WJavaScriptPreamble(JavaScriptScope scope,
					 JavaScriptObjectType type,
					 const char *name, const char *src)
  : scope(scope),
    type(type),
    name(name),
    src(src)
{ }

}
