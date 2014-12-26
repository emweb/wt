#include <boost/test/unit_test.hpp>
#include <boost/assign/list_of.hpp>

#include <Wt/Render/Block.h>
#include <Wt/Render/CssParser.h>
#include <iostream>
#include "3rdparty/rapidxml/rapidxml.hpp"
using namespace boost::assign;
using namespace Wt::Render;

#include <boost/version.hpp>

#if !defined(WT_NO_SPIRIT) && BOOST_VERSION >= 104700
#  define CSS_PARSER
#endif

#ifdef CSS_PARSER

const Wt::Render::Block* childBlock(const Wt::Render::Block* parent,
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

Wt::rapidxml::xml_document<>* createXHtml(const char *xhtml)
{
  Wt::rapidxml::xml_document<>* doc = new Wt::rapidxml::xml_document<>();

  char *cxhtml = doc->allocate_string(xhtml);
  doc->parse<Wt::rapidxml::parse_xhtml_entity_translation>(cxhtml);

  return doc;
}

BOOST_AUTO_TEST_CASE( CssSelector_test1 )
{
  Wt::rapidxml::xml_document<>* doc = createXHtml(
        "<ul><li></li></ul><li><h1><h2><h1></h1></h2></h1></li>");


  Wt::Render::StyleSheet* style = Wt::Render::CssParser().parse(
        "ul{} ul li{} li{} ul h1{} h1 h1{} li h1 h1{}");

  BOOST_REQUIRE( style );


  Wt::Render::Block b(doc, 0);
  // PASS ul to "ul"
  BOOST_REQUIRE(  Match::isMatch(childBlock(&b, list_of(0)),
                  style->rulesetAt(0).selector() ) );
  // PASS ul/li to "ul li"
  BOOST_REQUIRE(  Match::isMatch(childBlock(&b, list_of(0)(0)),
                  style->rulesetAt(1).selector() ) );
  // PASS ul/li to "li"
  BOOST_REQUIRE(  Match::isMatch(childBlock(&b, list_of(0)(0)),
                  style->rulesetAt(2).selector() ) );
  // FAIL li to "ul li"
  BOOST_REQUIRE( !Match::isMatch(childBlock(&b, list_of(1)),
                  style->rulesetAt(1).selector() ) );
  // FAIL li/h1/h2/h1 to "ul h1"
  BOOST_REQUIRE( !Match::isMatch(childBlock(&b, list_of(1)(0)(0)(0)),
                  style->rulesetAt(3).selector() ) );
  // PASS li/h1/h2/h1 to "li h1 h1"
  BOOST_REQUIRE(  Match::isMatch(childBlock(&b, list_of(1)(0)(0)(0)),
                  style->rulesetAt(5).selector() ) );
  // PASS li/h1/h2/h1 to "h1 h1"
  BOOST_REQUIRE(  Match::isMatch(childBlock(&b, list_of(1)(0)(0)(0)),
                  style->rulesetAt(4).selector() ) );

  delete style;
  delete doc;
}

BOOST_AUTO_TEST_CASE( CssSelector_test2 )
{
  Wt::rapidxml::xml_document<>* doc = createXHtml(
        "<h1><h2 id=\"two\"><h3><h4 id=\"four\"></h4></h3></h2></h1>");

  Wt::Render::StyleSheet* style = Wt::Render::CssParser().parse(
        "#two #four{} #two h4#four{} #two h3#four{}");

  BOOST_REQUIRE( style );

  Wt::Render::Block b(doc, 0);
  // PASS h1/h2/h3/h4 to "#two #four"
  BOOST_REQUIRE(  Match::isMatch(childBlock(&b, list_of(0)(0)(0)(0)),
                  style->rulesetAt(0).selector() ) );
  // PASS h1/h2/h3/h4 to "#two h4#four"
  BOOST_REQUIRE(  Match::isMatch(childBlock(&b, list_of(0)(0)(0)(0)),
                  style->rulesetAt(1).selector() ) );
  // FAIL h1/h2/h3/h4 to "#two h3#four"
  BOOST_REQUIRE( !Match::isMatch(childBlock(&b, list_of(0)(0)(0)(0)),
                  style->rulesetAt(2).selector() ) );
  // FAIL h1/h2/h3 to "#two h3#four"
  BOOST_REQUIRE( !Match::isMatch(childBlock(&b, list_of(0)(0)(0)),
                  style->rulesetAt(2).selector() ) );

  delete style;
  delete doc;
}

BOOST_AUTO_TEST_CASE( CssSelector_test3 )
{
  Wt::rapidxml::xml_document<>* doc = createXHtml(
 "<h1><h2 class=\"b\"><h3 class=\"c e\"><h4 class=\"d\"></h4></h3></h2></h1>");

  Wt::Render::StyleSheet* style = Wt::Render::CssParser().parse(
        ".b .d{} .b h4.d{} .b h3.d{} .b .c.e .d{}");

  BOOST_REQUIRE( style );

  Wt::Render::Block b(doc, 0);
  // PASS h1/h2/h3/h4 to ".b .d"
  BOOST_REQUIRE(  Match::isMatch(childBlock(&b, list_of(0)(0)(0)(0)),
                  style->rulesetAt(0).selector() ) );
  // PASS h1/h2/h3/h4 to ".b h4.d"
  BOOST_REQUIRE(  Match::isMatch(childBlock(&b, list_of(0)(0)(0)(0)),
                  style->rulesetAt(1).selector() ) );
  // FAIL h1/h2/h3/h4 to ".b h3.d"
  BOOST_REQUIRE( !Match::isMatch(childBlock(&b, list_of(0)(0)(0)(0)),
                  style->rulesetAt(2).selector() ) );
  // FAIL h1/h2/h3 to ".b h3.d"
  BOOST_REQUIRE( !Match::isMatch(childBlock(&b, list_of(0)(0)(0)),
                  style->rulesetAt(2).selector() ) );
  // PASS h1/h2/h3/h4 to ".b .c.e .d"
  BOOST_REQUIRE(  Match::isMatch(childBlock(&b, list_of(0)(0)(0)(0)),
                  style->rulesetAt(3).selector() ) );

  delete style;
  delete doc;
}

BOOST_AUTO_TEST_CASE( CssSelector_test4 )
{
  Wt::rapidxml::xml_document<>* doc = createXHtml(
 "<h1 class=\"a1 a2\" id=\"one\"><h2><h3 class=\"c\" id=\"two\"></h3></h2></h1>"
        );

  Wt::Render::StyleSheet* style = Wt::Render::CssParser().parse(
        "h1.a1#one.a2 * h3#two.c{}");

  BOOST_REQUIRE( style );

  Wt::Render::Block b(doc, 0);
  // PASS h1/h2/h3 to "h1.a1#one.a2 * h3#two.c{}"
  BOOST_REQUIRE(  Match::isMatch(childBlock(&b, list_of(0)(0)(0)),
                  style->rulesetAt(0).selector() ) );

  delete style;
  delete doc;
}

BOOST_AUTO_TEST_CASE( CssSelector_testSpecificity )
{
  Wt::Render::StyleSheet* style = Wt::Render::CssParser().parse(
        "h1, *, h1.a1#one.a2 * h3#two.c h4 h5{}");
  BOOST_REQUIRE( style );
  BOOST_REQUIRE( style->rulesetAt(0).selector().specificity()
                 == Wt::Render::Specificity(0,0,0,1) );
  BOOST_REQUIRE( style->rulesetAt(1).selector().specificity()
                 == Wt::Render::Specificity(0,0,0,0) );
  BOOST_REQUIRE( style->rulesetAt(2).selector().specificity()
                 == Wt::Render::Specificity(0,2,3,4) );
  delete style;
}

BOOST_AUTO_TEST_CASE( CssSelector_test5 )
{
  Wt::rapidxml::xml_document<>* doc = createXHtml(
        "<h1><h1><h1></h1></h1></h1>");

  Wt::Render::StyleSheet* style = Wt::Render::CssParser().parse(
        "h1 h1 h1{}");

  BOOST_REQUIRE( style );


  Wt::Render::Block b(doc, 0);
  // Sanity check, match h1/h1/h1 to "h1 h1 h1"
  BOOST_REQUIRE(  Match::isMatch(childBlock(&b, list_of(0)(0)(0)),
                  style->rulesetAt(0).selector() ) );
  // FAIL h1/h1 to "h1 h1 h1"
  BOOST_REQUIRE( !Match::isMatch(childBlock(&b, list_of(0)(0)),
                  style->rulesetAt(0).selector() ) );
}

#endif // CSS_PARSER
