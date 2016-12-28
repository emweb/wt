
#include <boost/test/unit_test.hpp>

#include <Wt/Render/CssParser.h>
#include <iostream>

#include <boost/version.hpp>

#if !defined(WT_NO_SPIRIT) && BOOST_VERSION >= 104700
#  define CSS_PARSER
#endif

#ifdef CSS_PARSER

bool isValid(std::unique_ptr<Wt::Render::StyleSheet> s)
{
  return s != nullptr;
}

BOOST_AUTO_TEST_CASE( CssParser_test1 )
{
  Wt::Render::CssParser parser;
  {
    auto s  = parser.parse("h1 { color: green }  h1 h2, h1 h3{color: red}");
    BOOST_REQUIRE( s );
    BOOST_REQUIRE( s->rulesetSize() == 3 );
    BOOST_REQUIRE( s->rulesetAt(1).selector().size() == 2 );
    BOOST_REQUIRE( s->rulesetAt(2).selector().size() == 2 );
  }

  {
    auto s2 = parser.parse("h1 h2 h3 h4 {color: green}");
    BOOST_REQUIRE( s2 );
    BOOST_REQUIRE( s2->rulesetSize() == 1 );
    BOOST_REQUIRE( s2->rulesetAt(0).selector().size()  == 4 );
  }

  BOOST_REQUIRE( !isValid(parser.parse("h1 h2 h3 & h4 {inside: ok}")) );
  BOOST_REQUIRE(  isValid(parser.parse("h1{}")) );
  BOOST_REQUIRE( !isValid(parser.parse("1h{}")) );
  BOOST_REQUIRE( !isValid(parser.parse("h 1{}")) );
  BOOST_REQUIRE(  isValid(parser.parse(".class1{}")) );
  BOOST_REQUIRE( !isValid(parser.parse(".1class{}")) );
  BOOST_REQUIRE(  isValid(parser.parse("#id1{}")) );
  BOOST_REQUIRE( !isValid(parser.parse("#1id{}")) );
  BOOST_REQUIRE( !isValid(parser.parse("{}")) );
  BOOST_REQUIRE(  isValid(parser.parse("id_{id_:boo}")) );
  {
    auto s6 = parser.parse("a{inside:\"}b{\"}");
    BOOST_REQUIRE(  s6 );
    BOOST_REQUIRE(  s6->rulesetSize() == 1 );
    BOOST_REQUIRE(  isValid(parser.parse("h1{ a: a; b: b }")) );
    BOOST_REQUIRE(  isValid(parser.parse("h1{ a: a; b: b; }")) );
    BOOST_REQUIRE(  isValid(parser.parse("h1{ a: 2em }")) );
  }

  {
    auto s3 = parser.parse(".class1.class2{}");
    BOOST_REQUIRE( s3 );
    BOOST_REQUIRE( s3->rulesetSize() == 1 );
    BOOST_REQUIRE( s3->rulesetAt(0).selector().size() == 1 );
  }

  {
    auto s4 = parser.parse(".class1 .class2{}");
    BOOST_REQUIRE( s4 );
    BOOST_REQUIRE( s4->rulesetSize() == 1 );
    BOOST_REQUIRE( s4->rulesetAt(0).selector().size() == 2 );
  }

  {
    auto s5 = parser.parse(
	  "h1{color: 20px; something: blue; something_else: \"bla\" }");
    BOOST_REQUIRE( s5 );
    BOOST_REQUIRE( s5->rulesetSize() == 1 );
    /*BOOST_REQUIRE( s5->rulesetAt(0).declarationBlock().value("color").value_
				    == 20.0 );
    BOOST_REQUIRE( s5->rulesetAt(0).declarationBlock().value("color").unit_
				    == Wt::Render::Term::Px );
    BOOST_REQUIRE( s5->rulesetAt(0).declarationBlock().value("something")
				   .identifier_ == "blue" );
    BOOST_REQUIRE( s5->rulesetAt(0).declarationBlock().value("something_else")
				   .quotedString_ == "bla" );*/
  }

  // Test hex color
  BOOST_REQUIRE(  isValid(parser.parse("h1{color:#123}")) );
  BOOST_REQUIRE(  isValid(parser.parse("h1{color:#a11}")) );
  BOOST_REQUIRE(  isValid(parser.parse("h1{color:#123456}")) );

  // Test multi term expressions
  BOOST_REQUIRE( isValid(parser.parse("h1{test: .1px}")) );
  BOOST_REQUIRE( isValid(parser.parse("hr{border: 1px 1px}")) );
  BOOST_REQUIRE( isValid(parser.parse("hr{outline: thin dotted invert}")) );

  // Test comments
  BOOST_REQUIRE( isValid(parser.parse("/*bla*/ h1{}")) );
  BOOST_REQUIRE( isValid(parser.parse("h1{/*bla*/ test: 1px}")) );
  BOOST_REQUIRE( isValid(parser.parse("h1{test: /*bla*/1px}")) );
  BOOST_REQUIRE( isValid(parser.parse("h1{test: /*bla* */1px}")) );
  BOOST_REQUIRE( isValid(parser.parse("<!--bla--> h1{test: 1px}")) );
  BOOST_REQUIRE( isValid(parser.parse("h1{test: 1px <!--bla--> }")) );
  BOOST_REQUIRE( isValid(parser.parse("h1{test: 1px <!--b-l-a--> }")) );

  // Test white spaces
  BOOST_REQUIRE( isValid(parser.parse("h1{test:\t\n\r\f 1px}")) );

  // Test URI's
  BOOST_REQUIRE( isValid(parser.parse("h1{test:url(\'bla\')}")) );
  BOOST_REQUIRE( isValid(parser.parse("h1{test:url(\"bla\")}")) );
  BOOST_REQUIRE( isValid(parser.parse("h1{test:url(bla)}")) );
  BOOST_REQUIRE( isValid(parser.parse("h1{test:url( \"bla\" )}")) );
  BOOST_REQUIRE( isValid(parser.parse("h1{test:url( \t\n\f\r \"bla\" )}")) );
  BOOST_REQUIRE( isValid(parser.parse("h1{test:url(\"folder\\image.gif\" )}")) );

}

BOOST_AUTO_TEST_CASE( CssParser_testDefaultStylesheet )
{
  Wt::Render::CssParser parser;
  BOOST_REQUIRE( !isValid(parser.parseFile("")) );
  BOOST_REQUIRE(  isValid(parser.parseFile("../resources/html4_default.css")) );
}

#endif // CSS_PARSER
