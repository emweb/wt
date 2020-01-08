/*
 * Copyright (C) 2012 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "RenderUtils.h"

#include "Block.h"

using namespace Wt::rapidxml;

namespace Wt {
  namespace Render {
    namespace Utils {
      bool normalizeWhitespace(Wt::Render::Block *block, 
			       Wt::rapidxml::xml_node<> *node,
			       bool haveWhitespace, 
			       Wt::rapidxml::xml_document<>& doc)
      {
	Wt::rapidxml::memory_pool<>& pool = doc;
      
	char *v = node->value();
      
	unsigned len = node->value_size();
      
	std::string s;
	s.reserve(len);
      
	for (unsigned i = 0; i < len; ++i) {
	  if (block->isWhitespace(v[i])) {
	    if (!haveWhitespace)
	      s += ' ';
	    haveWhitespace = true;
	  } else if (i < len - 1 && 
		     v[i] == (char)0xC2 && v[i+1] == (char)0xA0) {
	    /*
	     * This wrong but will work temporarily. We are treating &nbsp;
	     * (resolved to UTF-8 0xC2 0xA0) here equal to normal space.
	     */
	    if (!haveWhitespace)
	      s += ' ';
	    haveWhitespace = true;
	    ++i;
	  } else {
	    s += v[i];
	    haveWhitespace = false;
	  }
	}
      
	char *nv = pool.allocate_string(s.c_str(), s.length());
	node->value(nv, s.length());

	return haveWhitespace;
      }

      bool isXMLElement(xml_node<> *node)
      {
	return node->type() == node_element;
      }

      void fetchBlockChildren(xml_node<> *node, 
			      Wt::Render::Block *block, 
			      std::vector<Wt::Render::Block *> &children)
      {
	for (xml_node<> *child = node->first_node(); child;
	     child = child->next_sibling())
	  children.push_back(new Wt::Render::Block(child, block));
      }

      std::string nodeValueToString(Wt::rapidxml::xml_node<> *node)
      {
	return std::string(node->value(), node->value_size());
      }
    }
  }
}
