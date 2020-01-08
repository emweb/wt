/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WApplication.h"
#include "Wt/WEnvironment.h"
#include "Wt/WLogger.h"
#include "Wt/WString.h"
#include "Wt/WStringStream.h"

#include "DomElement.h"
#include "RefEncoder.h"
#include "WebSession.h"
#include "WebUtils.h"

#include <regex>
#include <iterator>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include "3rdparty/rapidxml/rapidxml.hpp"
#include "3rdparty/rapidxml/rapidxml_print.hpp"

using namespace Wt::rapidxml;

namespace Wt {

LOGGER("RefEncoder");

static std::string replaceUrlInStyle(std::string& style, WApplication *app)
{
  std::regex re("url\\((.*//.*)\\)", std::regex::icase);

  std::sregex_iterator i(style.begin(), style.end(), re);
  std::sregex_iterator end;

  WStringStream result;
  std::size_t pos = 0;

  for (; i != end; ++i) {
    result << style.substr(pos, i->position(1) - pos);

    std::string url = style.substr(i->position(1), i->length(1));
    boost::algorithm::trim(url);
    if (url.length() > 2)
      if (url[0] == '\'' || url[1] == '"')
	url = url.substr(1, url.length() - 2);
    
    result << WWebWidget::jsStringLiteral(app->encodeUntrustedUrl(url), '\'');

    pos = i->position(1) + i->length(1);
  }

  result << style.substr(pos);

  return result.str();
}

void EncodeRefs(xml_node<> *x_node, WApplication *app,
		WFlags<RefEncoderOption> options)
{
  xml_document<> *doc = x_node->document();

  if (strcmp(x_node->name(), "a") == 0) {
    xml_attribute<> *x_href = x_node->first_attribute("href");
    if (x_href) {
      std::string path = x_href->value();
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
	if (path.find("://") != std::string::npos ||
	    boost::starts_with(path, "//")) {
	  path = app->encodeUntrustedUrl(path);
	  x_href->value(doc->allocate_string(path.c_str()));
	}
      }
    }
  }

  if (options & EncodeRedirectTrampoline) {
    xml_attribute<> *x_style = x_node->first_attribute("style");

    if (x_style) {
      std::string style = x_style->value();
      if (style.find("//") != std::string::npos) {
        style = replaceUrlInStyle(style, app);
        x_style->value(doc->allocate_string(style.c_str()));
      }
    }

    if (strcmp(x_node->name(), "img") == 0) {
      xml_attribute<> *x_scr = x_node->first_attribute("src");
      if (x_scr) {
        std::string path = x_scr->value();
        if (path.find("://") != std::string::npos ||
	    boost::starts_with(path, "//")) {
          path = app->encodeUntrustedUrl(path);
          x_scr->value(doc->allocate_string(path.c_str()));
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

WString EncodeRefs(const WString& text, WFlags<RefEncoderOption> options)
{
  if (text.empty())
    return text;

  std::string result = "<span>" + text.toXhtmlUTF8() + "</span>";
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
    return text;
  }

  if (result.length() < 13)
    result.clear();
  else
    result = result.substr(6, result.length() - 13);

  return WString::fromUTF8(result);
}

}
