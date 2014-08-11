#include <boost/test/unit_test.hpp>
#include <boost/assign/list_of.hpp>
#include <Wt/Render/Block.h>
#include <Wt/Render/CssParser.h>

#include <boost/version.hpp>

#if !defined(WT_NO_SPIRIT) && BOOST_VERSION >= 104700
#  define CSS_PARSER
#endif

#ifdef CSS_PARSER

using namespace boost::assign;

const Wt::Render::Block* childBlock2(const Wt::Render::Block* parent,
                                    const std::vector<int>& indexes)
{
  const Wt::Render::Block* block = parent;
  for(std::vector<int>::const_iterator iter = indexes.begin();
      iter != indexes.end(); ++iter)
  {
    block = block->children().at(*iter);
  }
  return block;
}

Wt::rapidxml::xml_document<>* createXHtml2(const char * xhtml)
{
  Wt::rapidxml::xml_document<>* doc = new Wt::rapidxml::xml_document<>();
  char *cxhtml = doc->allocate_string(xhtml);
  doc->parse<Wt::rapidxml::parse_xhtml_entity_translation>(cxhtml);

  return doc;
}

BOOST_AUTO_TEST_CASE( BlockCssProperty_test1 )
{
  Wt::rapidxml::xml_document<>* doc = createXHtml2(
        "<h2>"
          "<h3>"
          "</h3>"
        "</h2>"
        "<h1>"
          "<h2>"
            "<h3>"
              "<h4>"
              "</h4>"
            "</h3>"
          "</h2>"
        "</h1>");


  Wt::Render::StyleSheet* style = Wt::Render::CssParser().parse(
        "h1{border: 1px}"
        "h2{border: 2px}"
        "h1 h2{border: 12px }"
        "h1 h2{border-right: 120px }"
        "h2{border: 20px}"
        "h1{color: blue}"
        );

  BOOST_REQUIRE( style );

  Wt::Render::Block b(doc, 0);
  b.setStyleSheet(style);
  // h1 color == "blue"
  BOOST_REQUIRE(childBlock2(&b, list_of(1))
                ->cssProperty(Wt::PropertyStyleColor) == "blue");
  // h1 border-left == 1px
  BOOST_REQUIRE(childBlock2(&b, list_of(1))
                ->cssProperty(Wt::PropertyStyleBorderLeft) == "1px");
  // h1/h2 border-left == 12px
  BOOST_REQUIRE(childBlock2(&b, list_of(1)(0))
                ->cssProperty(Wt::PropertyStyleBorderLeft) == "12px");
  // h2 border-left == 20px
  BOOST_REQUIRE(childBlock2(&b, list_of(0))
                ->cssProperty(Wt::PropertyStyleBorderLeft) == "20px");
  // h1/h2 border-right == 120px
  BOOST_REQUIRE(childBlock2(&b, list_of(1)(0))
                ->cssProperty(Wt::PropertyStyleBorderRight) == "120px");

  delete style;
  delete doc;
}

#endif // CSS_PARSER
