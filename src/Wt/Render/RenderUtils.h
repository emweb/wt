// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2012 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef RENDER_UTILS_H_
#define RENDER_UTILS_H_

#include <string>
#include <vector>

#include "3rdparty/rapidxml/rapidxml.hpp"

#ifdef WT_TARGET_JAVA
#include <hpdf.h>
#endif

namespace Wt {
  namespace Render {
    class Block;
    namespace Utils {
      bool normalizeWhitespace(Wt::Render::Block *block,
			       Wt::rapidxml::xml_node<> *node,
			       bool haveWhitespace, 
			       Wt::rapidxml::xml_document<>& doc);
      bool isXMLElement(Wt::rapidxml::xml_node<> *node);
      void fetchBlockChildren(Wt::rapidxml::xml_node<> *node, 
			      Wt::Render::Block *block, 
			      std::vector<Wt::Render::Block *> &children);
      std::string nodeValueToString(Wt::rapidxml::xml_node<> *node);

#ifdef WT_TARGET_JAVA
      HPDF_Page createPage(HPDF_Doc pdf, double width, double height);
#endif
    }
  }
}

#endif // RENDER_UTILS_H_
