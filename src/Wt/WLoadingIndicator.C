/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WLoadingIndicator"

namespace Wt {
#ifndef WT_TARGET_JAVA
  WLoadingIndicator::WLoadingIndicator(WObject *parent):
	WObject(parent) 
  {

  }
#endif //WT_TARGET_JAVA

  WLoadingIndicator::~WLoadingIndicator() {
  }

}
