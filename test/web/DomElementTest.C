/*
 * Copyright (C) 2023 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/WProgressBar.h"
#include <boost/test/unit_test.hpp>

#include <web/DomElement.h>

BOOST_AUTO_TEST_CASE( css_name_test )
{
  std::string cssNames[] =
  { "position", "z-index", "float", "clear", "width", "height", "line-height", "min-width", "min-height",
    "max-width", "max-height", "left", "right", "top", "bottom", "inset-inline", "inset-inline-start",
    "inset-inline-end", "inset-block", "inset-block-start", "inset-block-end", "vertical-align", "text-align",
    "padding", "padding-top", "padding-right", "padding-bottom", "padding-left", "padding-inline",
    "padding-inline-start", "padding-inline-end", "padding-block", "padding-block-start", "padding-block-end",
    "margin", "margin-top", "margin-right", "margin-bottom", "margin-left", "margin-inline",
    "margin-inline-start", "margin-inline-end", "margin-block", "margin-block-start", "margin-block-end",
    "cursor", "border-top", "border-right", "border-bottom", "border-left", "border-inline",
    "border-inline-start", "border-inline-end", "border-block", "border-block-start", "border-block-end",
    "border-color-top", "border-color-right", "border-color-bottom", "border-color-left", "border-inline-color",
    "border-inline-start-color", "border-inline-end-color", "border-block-color", "border-block-start-color",
    "border-block-end-color", "border-width-top", "border-width-right", "border-width-bottom",
    "border-width-left", "border-inline-width", "border-inline-start-width", "border-inline-end-width",
    "border-block-width", "border-block-start-width", "border-block-end-width", "color", "overflow-x",
    "overflow-y", "opacity", "font-family", "font-style", "font-variant", "font-weight", "font-size",
    "background-color", "background-image", "background-repeat", "background-attachment",
    "background-position", "text-decoration", "white-space", "table-layout", "border-spacing",
    "border-collapse", "page-break-before", "page-break-after", "zoom", "visibility", "display",
    "-webkit-appearance", "box-sizing", "flex", "flex-direction", "flex-flow", "align-self", "justify-content",
    "anchor-name", "position-anchor", "position-area", "position-try-fallbacks"
  };

  for (int i = static_cast<int>(Wt::Property::StylePosition); i < static_cast<int>(Wt::Property::LastPlusOne); ++i) {
    BOOST_REQUIRE_EQUAL(Wt::DomElement::cssName(static_cast<Wt::Property>(i)), cssNames[i - static_cast<int>(Wt::Property::StylePosition)]);
  }
}

BOOST_AUTO_TEST_CASE( css_camel_name_test )
{
  std::string cssCamelNames[] =
  { "cssText", "width", "position", "zIndex", "cssFloat", "clear", "width", "height", "lineHeight",
    "minWidth", "minHeight", "maxWidth", "maxHeight", "left", "right", "top", "bottom", "insetInline",
    "insetInlineStart", "insetInlineEnd", "insetBlock", "insetBlockStart", "insetBlockEnd", "verticalAlign",
    "textAlign", "padding", "paddingTop", "paddingRight", "paddingBottom", "paddingLeft", "paddingInline",
    "paddingInlineStart", "paddingInlineEnd", "paddingBlock", "paddingBlockStart", "paddingBlockEnd",
    "margin", "marginTop", "marginRight", "marginBottom", "marginLeft", "marginInline", "marginInlineStart",
    "marginInlineEnd", "marginBlock", "marginBlockStart", "marginBlockEnd", "cursor", "borderTop",
    "borderRight", "borderBottom", "borderLeft", "borderInline", "borderInlineStart", "borderInlineEnd",
    "borderBlock", "borderBlockStart", "borderBlockEnd", "borderColorTop", "borderColorRight",
    "borderColorBottom", "borderColorLeft", "borderInlineColor", "borderInlineStartColor",
    "borderInlineEndColor", "borderBlockColor", "borderBlockStartColor", "borderBlockEndColor",
    "borderWidthTop", "borderWidthRight", "borderWidthBottom", "borderWidthLeft", "borderInlineWidth",
    "borderInlineStartWidth", "borderInlineEndWidth", "borderBlockWidth", "borderBlockStartWidth",
    "borderBlockEndWidth", "color", "overflowX", "overflowY", "opacity", "fontFamily", "fontStyle",
    "fontVariant", "fontWeight", "fontSize", "backgroundColor", "backgroundImage", "backgroundRepeat",
    "backgroundAttachment", "backgroundPosition", "textDecoration", "whiteSpace", "tableLayout",
    "borderSpacing", "border-collapse", "pageBreakBefore", "pageBreakAfter", "zoom", "visibility",
    "display", "webKitAppearance", "boxSizing", "flex", "flexDirection", "flexFlow", "alignSelf",
    "justifyContent", "anchorName", "positionAnchor", "positionArea", "positionTryFallbacks"
  };

  for (int i = static_cast<int>(Wt::Property::StylePosition); i < static_cast<int>(Wt::Property::LastPlusOne); ++i) {
    BOOST_REQUIRE_EQUAL(Wt::DomElement::cssJavaScriptName(static_cast<Wt::Property>(i)), cssCamelNames[i - static_cast<int>(Wt::Property::Style)]);
  }
}

BOOST_AUTO_TEST_CASE ( css_name_nonexistent_test )
{
  BOOST_REQUIRE_EQUAL(Wt::DomElement::cssName(Wt::Property::Style), "");
}

BOOST_AUTO_TEST_CASE ( css_camel_name_nonexistent_test )
{
  BOOST_REQUIRE_EQUAL(Wt::DomElement::cssJavaScriptName(Wt::Property::Src), "");
}
