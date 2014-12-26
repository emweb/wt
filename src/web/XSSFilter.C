/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WLogger"
#include "Wt/WString"
#include "Wt/WStringStream"

#include "DomElement.h"
#include "XSSUtils.h"

#include "3rdparty/rapidxml/rapidxml.hpp"
#include "3rdparty/rapidxml/rapidxml_print.hpp"

using namespace Wt::rapidxml;

namespace Wt {

LOGGER("XSS");

void XSSSanitize(xml_node<> *x_node)
{
  for (xml_attribute<> *x_attr = x_node->first_attribute(); x_attr;) {

    xml_attribute<> *x_next_attr = x_attr->next_attribute();
    if (Wt::XSS::isBadAttribute(x_attr->name())
	|| Wt::XSS::isBadAttributeValue(x_attr->name(), x_attr->value())) {
      LOG_SECURE("discarding invalid attribute: "
		 << x_attr->name() << ": " << x_attr->value());
      x_node->remove_attribute(x_attr);
    }

    x_attr = x_next_attr;
  }

  for (xml_node<> *x_child = x_node->first_node(); x_child;) {
    xml_node<> *x_next_child = x_child->next_sibling();

    if (Wt::XSS::isBadTag(x_child->name())) {
      LOG_SECURE("discarding invalid tag: " << x_child->name());
      x_node->remove_node(x_child);
    } else
      XSSSanitize(x_child);

    x_child = x_next_child;
  }

  if (!x_node->first_node()
      && x_node->value_size() == 0
      && !DomElement::isSelfClosingTag(x_node->name())) {
    // We need to add an emtpy data node since <div /> is illegal HTML
    // (but valid XML / XHTML)
    xml_node<> *empty
      = x_node->document()->allocate_node(node_data, 0, 0, 0, 0);
    x_node->append_node(empty);
  }
}

bool XSSFilterRemoveScript(WString& text)
{
  if (text.empty())
    return true;

  std::string result = "<span>" + text.toUTF8() + "</span>";
  char *ctext = const_cast<char *>(result.c_str()); // Shhht it's okay !

  try {
    xml_document<> doc;
    doc.parse<parse_comment_nodes
      | parse_validate_closing_tags
      | parse_validate_utf8
      | parse_xhtml_entity_translation>(ctext);

    XSSSanitize(doc.first_node());

    WStringStream out;
    print(out.back_inserter(), *doc.first_node(), print_no_indenting);
    result = out.str();
  } catch (parse_error& e) {
    LOG_ERROR("Error reading XHTML string: " << e.what());
    return false;
  }

  if (result.length() < 13)
    result.clear();
  else
    result = result.substr(6, result.length() - 13);

  text = WString::fromUTF8(result);

  return true;
}

}
