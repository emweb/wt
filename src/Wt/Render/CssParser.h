// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2012 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef RENDER_CSSPARSER_H_
#define RENDER_CSSPARSER_H_

#include <iostream>

#include <Wt/WDllDefs.h>
#include <Wt/WString.h>
#include "Wt/Render/CssData.h"

namespace Wt {
namespace Render {

class WT_API CssParser
{
public:
  CssParser();

  std::unique_ptr<StyleSheet> parse(const WString& styleSheetContents);
  std::unique_ptr<StyleSheet> parseFile(const WString& filename);
  std::string getLastError() const;

private:
  std::string error_;
};

  }
}

#endif // RENDER_CSSPARSER_H_
