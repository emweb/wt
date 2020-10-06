#include "CssParser.h"

#include "CssData.h"
#include "CssData_p.h"

#include <boost/version.hpp>

using namespace Wt::Render;

#if !defined(WT_NO_SPIRIT) && BOOST_VERSION >= 104700
#  define CSS_PARSER
#endif

#ifdef CSS_PARSER

#include <algorithm>
#include <functional>

// Spirit (file_iterator) includes windows.h
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN

#include <boost/algorithm/string.hpp>
#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_repeat.hpp>
#include <boost/spirit/repository/include/qi_confix.hpp>
#include <boost/spirit/include/qi_real.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/variant/recursive_variant.hpp>
#include <boost/spirit/home/support/context.hpp>
#if BOOST_VERSION < 105600
#include <boost/spirit/home/phoenix.hpp>
#else
#include <boost/phoenix.hpp>
#endif
#include <boost/spirit/include/classic_file_iterator.hpp>

#include <map>

#include <Wt/Render/Block.h>

namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::standard;
namespace phoenix = boost::phoenix;

using namespace Wt::Render;

BOOST_FUSION_ADAPT_STRUCT(
    SimpleSelectorImpl,
    (std::string, elementName_)
)

BOOST_FUSION_ADAPT_STRUCT(
    SelectorImpl,
    (std::vector<SimpleSelectorImpl>, simpleSelectors_)
)

/*
BOOST_FUSION_ADAPT_STRUCT(
    Term,
    (double, value_)
    (Term::Unit, unit_)
    (Term::Type, type_)
)
*/

typedef std::map<std::string, Term> justForBoostFusion;
BOOST_FUSION_ADAPT_STRUCT(
    DeclarationBlockImpl,
    (justForBoostFusion, properties_)
)

BOOST_FUSION_ADAPT_STRUCT(
    RulesetImpl,
    (SelectorImpl,              selector_)
    (std::string,               block_)
)

///////////////////////////////////////////////////////////////////////////////
///// CssSkipper                                                          /////
///////////////////////////////////////////////////////////////////////////////

template<typename Iterator>
struct CssSkipper : public qi::grammar<Iterator> {
    CssSkipper() : CssSkipper::base_type(skip_, "PL/0") {
        skip_ = ascii::space
                | ("/*" >> *(qi::char_ - "*/") >> "*/")
                | ("<!--" >> *(qi::char_ - "-->") >> "-->");
    }
    qi::rule<Iterator> skip_;
};

///////////////////////////////////////////////////////////////////////////////
///// DoubleWithoutExponent                                               /////
///////////////////////////////////////////////////////////////////////////////

//template <typename T>
struct DoubleWithoutExponent : qi::real_policies<double>
{
  template <typename Iterator>
  static bool parse_exp(Iterator&, Iterator const&)
  {
     return false;
  }
};

///////////////////////////////////////////////////////////////////////////////
///// CssGrammar                                                          /////
///////////////////////////////////////////////////////////////////////////////

template <typename Iterator>
class CssGrammar : qi::grammar<Iterator, CssSkipper<Iterator> >
{
  public:
    typedef CssGrammar<Iterator > Self;

    CssGrammar();
    bool parse(Iterator begin, Iterator end, StyleSheetImpl* s);

  //private:
    StyleSheetImpl* s_;
    void pushRulesetArray   ();
    void setCurrentSelectors(const std::vector<SelectorImpl>& selectors);
    void addDeclaration     (const std::string& property, const Term& term);
    void setDeclarationString(const std::string& rawstring);

    std::vector<RulesetImpl> currentRuleset_;

    qi::real_parser<double, DoubleWithoutExponent > doubleWithoutExponent_;
    qi::rule<Iterator,                             CssSkipper<Iterator> >
        ruleset_, rulesetArray_;
    qi::rule<Iterator, SelectorImpl(),             CssSkipper<Iterator> >
        selector_;
    qi::rule<Iterator,                             CssSkipper<Iterator> >
        declaration_;
    qi::rule<Iterator, std::vector<Term>(),        CssSkipper<Iterator> >
        expr_;
    qi::rule<Iterator, Term()                                       >
        term_;
    qi::rule<Iterator,                             CssSkipper<Iterator> >
        operator_;
    qi::rule<Iterator,                             CssSkipper<Iterator> >
        unary_operator_;
    qi::rule<Iterator, SimpleSelectorImpl()                         >
        simple_selector_;
    qi::rule<Iterator, std::string()>
        class_, HASH_, IDENT_, STRING_, nonstring_, nonascii_, nmstart_,
      element_name_, escape_, nmchar_, subset1_, subset2_;
    Iterator begin_;
    std::string error_;
};

struct fs_error_tag {};

template< typename Iterator >
struct ErrorReporting
{
#if BOOST_VERSION < 105600
  template< typename, typename, typename, typename, typename > // Phoenix v2
  struct result { typedef void type;};
#else
  typedef void result_type;
#endif

  ErrorReporting(CssGrammar< Iterator >* grammar)
    : grammar_(grammar)
  {}

  template< typename Info >
  void operator()(const Iterator& endPos,
                  const Iterator& errorPos,
                  Info info,
                  const std::string& filename,
                  fs_error_tag ) const
  {
    int line = std::count_if(grammar_->begin_, errorPos,
                             boost::is_any_of("\n\r\f")) + 1;

    Iterator lastNewLine = std::find_if(
          std::reverse_iterator< Iterator >(errorPos),
          std::reverse_iterator< Iterator >(grammar_->begin_),
          boost::is_any_of("\n\r\f")).base();

    unsigned int column = std::distance(lastNewLine, errorPos);

    std::string s(errorPos, (endPos - errorPos > 30) ? errorPos + 30 : endPos);
    std::replace_if(s.begin(), s.end(), boost::is_any_of("\n\r\f"), ' ');
    std::stringstream ss;
    ss
        << filename << ":" << line << ":" << column
        << ": Expecting " << info << " before \""
        << s << "\"" << std::endl;
    grammar_->error_ += ss.str();
  }

  CssGrammar< Iterator >* grammar_;
};

template< typename T >
std::vector< T >& operator+=( std::vector< T >& target,
                              const std::vector< T >& source )
{
  target.insert(target.end(), source.begin(), source.end());
  return target;
}

template<typename Iterator>
CssGrammar<Iterator>::CssGrammar()
 : CssGrammar::base_type(rulesetArray_, "CSS")
{
  using qi::lit;
  using qi::double_;
  using qi::lexeme;
  using qi::no_case;
  using qi::raw;
  using qi::on_error;
  using qi::fail;
  using qi::uint_parser;
  using qi::_val;
  using qi::_1;
  using qi::_2;
  using qi::_3;
  using qi::_4;
  using qi::repeat;
  using ascii::char_;
  using ascii::graph;
  using ascii::alnum;
  using ascii::alpha;

  rulesetArray_
    = +(ruleset_[phoenix::bind(&Self::pushRulesetArray, this)])
    ;

  ruleset_
    = (
         (selector_ % ',')[phoenix::bind(&Self::setCurrentSelectors, this, _1)]
      >  lit('{')
      >  raw[(-declaration_
          >  *(';' >> declaration_)
          >  -lit(';')
         )][phoenix::bind(&Self::setDeclarationString,
                          this,
                          boost::phoenix::construct<std::string>(
                            boost::phoenix::begin(_1),
                            boost::phoenix::end(_1)) )]
      >  lit('}')
      );

  selector_
    = (+simple_selector_
                    [phoenix::bind(&SelectorImpl::addSimpleSelector, _val, _1)])
    ;

  simple_selector_
    = (
           element_name_
                  [phoenix::bind(&SimpleSelectorImpl::setElementName, _val, _1)]
        >  *(  HASH_
                  [phoenix::bind(&SimpleSelectorImpl::setHash,        _val, _1)]
             | class_
                  [phoenix::bind(&SimpleSelectorImpl::addClass,       _val, _1)]
            )
      )
    |
      +(  HASH_   [phoenix::bind(&SimpleSelectorImpl::setHash,        _val, _1)]
        | class_  [phoenix::bind(&SimpleSelectorImpl::addClass,       _val, _1)]
       )
    ;

  declaration_
    = (IDENT_
    >  lit(":")
    >  expr_)
    ;

  expr_
    = +term_
    ;

  operator_
    = lit('/') | lit(',');

  term_
    = +(STRING_ | nonstring_) [phoenix::bind(&Term::setValue, _val, _1)];

  nonstring_ = ~char_("\"';}");
  
  element_name_
    = IDENT_
    | lit('*')
    ;

  class_
    = '.'
    >  IDENT_
    ;

  HASH_
    = '#'
    > IDENT_
    ;

  // TODO The following is a bit of a mess, but I couldn't get it
  // in 1 rule.
  subset1_ = ~char_("\n\r\f\\\"");
  subset2_ = ~char_("\n\r\f\\'");
  STRING_
    = (lit('\"') > *(escape_ | subset1_ | qi::string("\\\n")) > lit('\"'))
    | (lit('\'') > *(escape_ | subset2_ | qi::string("\\\n")) > lit('\''))
    ;

  nonascii_
    = char_("\xA0-\xFF")
    ;

  escape_
    = char_("\\")
    > ~char_("\r\n\f0-9a-fA-F")
    ;

  nmstart_
    = char_("_a-zA-Z")
    | nonascii_
    | escape_
    ;

  nmchar_
    = char_("_a-zA-Z0-9-")
    | nonascii_
    | escape_
    ;

  IDENT_
    = // '-' ???
      nmstart_
    > *nmchar_
    ;

  rulesetArray_.name("stylesheet");
  ruleset_.name("ruleset");
  selector_.name("selector");
  declaration_.name("declaration");
  expr_.name("expr");
  term_.name("term");
  operator_.name("operator");
  unary_operator_.name("unary_operator");
  class_.name("class");
  HASH_.name("HASH");
  IDENT_.name("IDENT");
  STRING_.name("STRING");
  nonstring_.name("nonstring");
  nonascii_.name("nonascii");
  nmstart_.name("nmstart");
  element_name_.name("element_name");
  escape_.name("escape");
  nmchar_.name("nmchar");
  subset1_.name("double_quoted_string");
  subset2_.name("single_quoted_string");

  phoenix::function<ErrorReporting< Iterator > > error_report(this);

  // _1: current iterator position
  // _2: end of input
  // _3: position of error
  // _4: info instance returned by what() called on the failing parser.
  on_error<qi::fail>
  (
    rulesetArray_,
    error_report(qi::_2, qi::_3, qi::_4, phoenix::val("styleSheetText()"), fs_error_tag())
  );

}

template <typename Iterator>
bool CssGrammar<Iterator>::parse(Iterator begin,
                                 Iterator end,
                                 StyleSheetImpl* s)
{
  CssSkipper<Iterator> skipper;

  begin_ = begin;
  s_ = s;
  bool success = phrase_parse(begin, end, rulesetArray_, skipper);
  // Annoying situation where the first simple_selector of a ruleset
  // is invalid
  if(success && begin != end)
  {
    ErrorReporting<Iterator >(this)
        (end, begin, "< ? >", "styleSheetText()", fs_error_tag());
  }

  return success && begin == end;
}

template <typename Iterator>
void CssGrammar<Iterator>::pushRulesetArray()
{
  s_->rulesetArray_ += currentRuleset_;
  currentRuleset_.clear();
}

template <typename Iterator>
void CssGrammar<Iterator>::setCurrentSelectors
                               (const std::vector<SelectorImpl>& selectors)
{
  BOOST_FOREACH(const SelectorImpl& s, selectors)
  {
    RulesetImpl ruleset;
    ruleset.selector_ = s;
    currentRuleset_.push_back(ruleset);
  }
}

template <typename Iterator>
void CssGrammar<Iterator>::addDeclaration(const std::string& property,
                                          const Term& term)
{
  BOOST_FOREACH(RulesetImpl& r, currentRuleset_)
    r.block_.properties_.insert(std::make_pair(property, term));
}

template <typename Iterator>
void CssGrammar<Iterator>::setDeclarationString(const std::string& rawstring)
{
  BOOST_FOREACH(RulesetImpl& r, currentRuleset_)
    r.block_.declarationString_ = rawstring;
}

///////////////////////////////////////////////////////////////////////////////
///// CssParser                                                           /////
///////////////////////////////////////////////////////////////////////////////
namespace Wt {
  namespace Render {

CssParser::CssParser()
{
}

std::unique_ptr<StyleSheet> CssParser::parse(const WString& styleSheetContents)
{
  error_.clear();
  std::unique_ptr<StyleSheetImpl> style = std::make_unique<StyleSheetImpl>();
  CssGrammar<std::string::const_iterator> cssGrammar;
  std::string s = styleSheetContents.toUTF8();
  bool success = cssGrammar.parse(s.begin(), s.end(), style.get());
  if (!success) {
    error_ = cssGrammar.error_;
    return nullptr;
  } else {
    error_ = "";
    return std::unique_ptr<StyleSheet>(std::move(style));
  }
}

std::unique_ptr<StyleSheet> CssParser::parseFile(const WString& filename)
{
  error_.clear();
  boost::spirit::classic::file_iterator<> first(filename.toUTF8());
  if(!first)
  {
    error_ = "file \"" + filename.toUTF8() + "\" not found";
    return nullptr;
  }
  boost::spirit::classic::file_iterator<> last = first.make_end();


  std::unique_ptr<StyleSheetImpl> style = std::make_unique<StyleSheetImpl>();
  CssGrammar<boost::spirit::classic::file_iterator<> > cssGrammar;
  bool success = cssGrammar.parse(first, last, style.get());
  if(!success)
  {
    error_ = cssGrammar.error_;
    return nullptr;
  }
  else
  {
    error_ = "";
    return std::unique_ptr<StyleSheet>(std::move(style));
  }
}

std::string CssParser::getLastError() const
{
  return error_;
}

  }
}

#else

namespace Wt {
  namespace Render {

CssParser::CssParser()
{ }

std::unique_ptr<StyleSheet> CssParser::parse(const WString& styleSheetContents)
{
  error_.clear();

  if (styleSheetContents.empty())
    return std::make_unique<StyleSheetImpl>();
  else {
    error_ = "Wt::Render: CSSParser requires Boost 1.47 or later";
    return nullptr;
  }
}

std::unique_ptr<StyleSheet> CssParser::parseFile(const WString& filename)
{
  error_ = "Wt::Render: CSSParser requires Boost 1.47 or later";
  return nullptr;
}

std::string CssParser::getLastError() const
{
  return error_;
}


  }
}

#endif // CSS_PARSER
