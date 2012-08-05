// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef XSS_FILTER_H
#define XSS_FILTER_H_

namespace Wt {

class WString;

extern bool XSSFilterRemoveScript(WString& text);

}

#endif // XSS_FILTER_H_
