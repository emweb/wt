/*
 * Copyright (C) 2011 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WApplication"
#include "Wt/WEnvironment"
#include "Wt/WLogger"
#include "Wt/WString"
#include "Wt/WStringStream"

#include "DomElement.h"
#include "RefEncoder.h"
#include "WebSession.h"
#include "WebUtils.h"

#include "rapidxml/rapidxml.hpp"
#include "rapidxml/rapidxml_print.hpp"

using namespace rapidxml;

namespace Wt {

LOGGER("RefEncoder");

void EncodeRefs(xml_node<> *x_node, WApplication *app,
		WFlags<RefEncoderOption> options)
{
  if (strcmp(x_node->name(), "a") == 0) {
    xml_attribute<> *x_href = x_node->first_attribute("href");
    if (x_href) {
      std::string path = x_href->value();
      xml_document<> *doc = x_node->document();

      if ((options & EncodeInternalPaths)
	  && path.length() >= 2 && path.substr(0, 2) == "#/") {
	path = path.substr(1);
	std::string addClass, url;

	if (app->environment().ajax()) {
	  url = app->bookmarkUrl(path);

	  const char *name = "onclick";
	  char *value
	    = doc->allocate_string
	    ((WT_CLASS".navigateInternalPath(event, "
	      + WWebWidget::jsStringLiteral(path) + ");").c_str());

	  xml_attribute<> *x_click = doc->allocate_attribute(name, value);
	  x_node->insert_attribute(0, x_click);

	  addClass = "Wt-rr";
	} else {
	  if (app->environment().agentIsSpiderBot())
	    url = app->bookmarkUrl(path);
	  else
	    url = app->session()->mostRelativeUrl(path);

	  addClass = "Wt-ip";
	}

	xml_attribute<> *x_class = x_node->first_attribute("class");
	std::string styleClass = x_class ? x_class->value() : std::string();

	styleClass = Utils::addWord(styleClass, addClass);

	if (x_class)
	  x_class->value(doc->allocate_string(styleClass.c_str()));
	else {
	  x_class = doc->allocate_attribute
	    ("class", doc->allocate_string(styleClass.c_str()));
	  x_node->insert_attribute(0, x_class);
	}

	x_href->value
	  (doc->allocate_string(app->resolveRelativeUrl(url).c_str()));
      } else if (options & EncodeRedirectTrampoline) {
	/*
	 * FIXME: also apply this to other occurences of URLs:
	 * - images
	 * - in CSS
	 */
	if (path.find("://") != std::string::npos) {
	  path = app->encodeUntrustedUrl(path);
	  x_href->value(doc->allocate_string(path.c_str()));
	}
      }
    }
  }

  for (xml_node<> *x_child = x_node->first_node(); x_child;
       x_child = x_child->next_sibling())
    EncodeRefs(x_child, app, options);

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

void EncodeRefs(WString& text, WFlags<RefEncoderOption> options)
{
  if (text.empty())
    return;

  std::string result = "<span>" + text.toUTF8() + "</span>";
  char *ctext = const_cast<char *>(result.c_str()); // Shhht it's okay !

  WApplication *app = WApplication::instance();

  try {
    xml_document<> doc;
    doc.parse<parse_comment_nodes
      | parse_validate_closing_tags
      | parse_validate_utf8
      | parse_xhtml_entity_translation>(ctext);

    EncodeRefs(doc.first_node(), app, options);

    WStringStream out;
    print(out.back_inserter(), *doc.first_node(), print_no_indenting);

    result = out.str();
  } catch (parse_error& e) {
    LOG_ERROR("Error reading XHTML string: " << e.what());
    return;
  }

  if (result.length() < 13)
    result.clear();
  else
    result = result.substr(6, result.length() - 13);

  text = WString::fromUTF8(result);
}

}
