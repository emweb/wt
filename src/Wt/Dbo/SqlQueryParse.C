/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Query.h"
#include "Query_impl.h"
#include "Exception.h"
#include "Logger.h"
#include "StringStream.h"
#include "ptr.h"

#include <boost/version.hpp>

#if !defined(WT_NO_SPIRIT) && defined(WT_CXX14) && BOOST_VERSION >= 106900 && (!defined(_MSC_VER) || _MSC_VER >= 1910)
#  define X3_QUERY_PARSE
#elif !defined(WT_NO_SPIRIT) && BOOST_VERSION >= 104100
#  define SPIRIT_QUERY_PARSE
#else
#  define NO_SPIRIT_QUERY_PARSE
#endif

#ifdef X3_QUERY_PARSE

#define BOOST_SPIRIT_X3_NO_FILESYSTEM

#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/support/utility/error_reporting.hpp>

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#endif // X3_QUERY_PARSE

#ifdef SPIRIT_QUERY_PARSE

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#if BOOST_VERSION < 105600
#include <boost/spirit/home/phoenix/statement/throw.hpp>
#else
#include <boost/phoenix.hpp>
#endif
#include <boost/bind/bind.hpp>
#include <iostream>

#endif // SPIRIT_QUERY_PARSE

#ifdef NO_SPIRIT_QUERY_PARSE

#include <boost/range/iterator_range.hpp>
#include <boost/algorithm/string.hpp>

#endif // NO_SPIRIT_QUERY_PARSE

namespace Wt {
  namespace Dbo {

LOGGER("Dbo.SqlQueryParse");

    namespace Impl {

#ifdef NO_SPIRIT_QUERY_PARSE
void parseSql(const std::string& sql, SelectFieldLists& fieldLists)
{
  fieldLists.clear();

  std::string::const_iterator i = sql.begin();

  std::size_t selectPos = ifind(sql, "select ");
  if (selectPos != 0)
    throw Exception("Session::query(): query should start with 'select '"
		    " (sql='" + sql + "')");

  i += 7;

  std::size_t distinctPos = ifind(sql.substr(i - sql.begin()), "distinct ");
  if (distinctPos == 0) {
    i += 9;
  } else {
    std::size_t allPos = ifind(sql.substr(i - sql.begin()), "all ");
    if (allPos == 0) {
      i += 4;
    }
  }

  std::string aliasStr;
  std::size_t fromPos = ifind(sql.substr(i - sql.begin()), " from ");
  if (fromPos != std::string::npos)
    aliasStr = sql.substr(i - sql.begin(), fromPos);
  else
    aliasStr = sql.substr(i - sql.begin());

  typedef std::vector<boost::iterator_range<std::string::iterator> >
    SplitVector;

  SplitVector aliases;
  boost::split(aliases, aliasStr, boost::is_any_of(","));

  fieldLists.push_back(SelectFieldList());

  int aliasStart = i - sql.begin();
  for (unsigned i = 0; i < aliases.size(); ++i) {
    std::string alias = std::string(aliases[i].begin(), aliases[i].end());
    std::string a1 = boost::trim_left_copy(alias);
    std::string a2 = boost::trim_right_copy(a1);

    SelectField f;
    f.begin = aliasStart + (aliases[i].begin() - aliasStr.begin())
      + alias.size() - a1.size();
    f.end = f.begin + a2.size();

    fieldLists.back().push_back(f);
  }
}

#endif // NO_SPIRIT_QUERY_PARSE

#ifdef SPIRIT_QUERY_PARSE

namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;
namespace phoenix = boost::phoenix;

/*
 * Grammar that parses SQL select statements:
 *
 *   http://www.sqlite.org/lang_select.html
 *   http://www.postgresql.org/docs/8.4/static/queries-with.html
 */
template <typename Iterator>
struct sql_query_grammar : qi::grammar<Iterator, ascii::space_type>
{
  typedef sql_query_grammar<Iterator> Self;

  sql_query_grammar(Iterator parseBegin, SelectFieldLists& fieldLists)
    : sql_query_grammar::base_type(query_expression),
      parseBegin_(parseBegin),
      fieldLists_(fieldLists)
  {
    using qi::lit;
    using qi::lexeme;
    using qi::no_case;
    using qi::raw;
    using qi::on_error;
    using qi::fail;
    using ascii::char_;
    using ascii::graph;
    using ascii::space;

    using phoenix::construct;
    using phoenix::val;
    using phoenix::throw_;

    query_expression 
      = select_expression % compound_operator
      ;

    select_expression 
      = with_clause
        >> no_case["select"][
			     boost::bind(&Self::handleSelect, this)
			     ]
	>> -( (no_case["distinct"] >>
	      -(no_case["on"] >> '(' >> (raw[field] % ',') >>  ')'))
	      | no_case["all"]
	      )
	>> fields
	>> -( no_case["from"] > from_clause )
      ;

    compound_operator
      = ( no_case["union"] >> -no_case["all"] )
      | no_case["intersect"]
      | no_case["except"]
      ;

    with_clause
      = *(sql_word - no_case["select"])
      ;

    from_clause
      = +(sql_word - compound_operator)
      ;

    fields
      = raw[field][
		   boost::bind(&Self::handleField, this, boost::arg<1>())
		   ] 
        % ','
      ;

    field
      = +(sub_expression | (identifier - lexeme[no_case["from"] >> +space]))
      ;

    sql_word
      = ',' | sub_expression | identifier
      ;

    sub_expression
      = '(' > *sql_word > ')'
      ;

    identifier
      = squoted
      | dquoted
      | other;

    squoted = lexeme[ '\'' > ( *(char_ - '\'') % "''" ) > '\'' ];
    dquoted = lexeme[ '"' > *(char_ - '"') > '"' ];
    other = lexeme[ +(graph - special) ];
    special = char_("()'\",");

    /*
    on_error<fail>
      (query_expression,
       throw_(construct<std::logic_error>
	      (val("Error parsing SQL query: Expected ")
	       + boost::spirit::_4
	       + val(" here: \"")
	       + construct<std::string>(boost::spirit::_3, boost::spirit::_2)
	       + val("\""))));
    */
    on_error<fail>
      (query_expression,
       [](boost::fusion::vector<
            Iterator&,
            Iterator const&,
            Iterator const&,
            qi::info const&> args,
       qi::unused_type, qi::unused_type) {
      if (Wt::Dbo::logging("error", Wt::Dbo::logger)) {
        boost::spirit::operator<<(
                  Wt::Dbo::log("error") <<
                  Wt::Dbo::logger << ": "
                  << "Error parsing SQL query: expected ",
                  boost::fusion::at_c<3>(args))
                  << " here: \""
                  << std::string(boost::fusion::at_c<2>(args),
                                 boost::fusion::at_c<1>(args))
                  << "\"\n";
      }
    });
  }

  Iterator parseBegin_;
  SelectFieldLists& fieldLists_;

  qi::rule<Iterator, ascii::space_type> query_expression,
    select_expression, compound_operator, with_clause, from_clause,
    fields, field, sql_word, sub_expression, identifier, squoted, dquoted,
    other;

  qi::rule<Iterator> special;

  void handleSelect()
  {
    fieldLists_.push_back(SelectFieldList());
  }
  
  void handleField(const boost::iterator_range<std::string::const_iterator>& s)
  {
    SelectField field;
    field.begin = s.begin() - parseBegin_;
    field.end = s.end() - parseBegin_;
    fieldLists_.back().push_back(field);
  }
};

void parseSql(const std::string& sql, SelectFieldLists& fieldLists)
{
  std::string::const_iterator iter = sql.begin();
  std::string::const_iterator end = sql.end();

  sql_query_grammar<std::string::const_iterator>
    sql_grammar(iter, fieldLists);

  bool success = qi::phrase_parse(iter, end, sql_grammar, ascii::space);

  if (success) {
    if (iter != end)
      throw Exception("Error parsing SQL query: Expected end here: \""
		      + std::string(iter, end) + "\"");
  } else
    throw Exception("Error parsing SQL query: \"" + sql + "\"");
}

#endif // SPIRIT_QUERY_PARSE

#ifdef X3_QUERY_PARSE

namespace x3 = boost::spirit::x3;

namespace sql_parser {

using FieldAttr = boost::iterator_range<std::string::const_iterator>;
using FieldsAttr = std::vector<FieldAttr>;
using DistinctClauseAttr = FieldsAttr;
using QueryExprAttr = std::vector<FieldsAttr>;

struct query_expression_class;

x3::rule<query_expression_class, QueryExprAttr> const query_expression = "query_expression";
x3::rule<class select_expression, FieldsAttr> const select_expression = "select_expression";
x3::rule<class distinct_clause, DistinctClauseAttr> const distinct_clause = "distinct_clause";
x3::rule<class compound_operator> const compound_operator = "compound_operator";
x3::rule<class with_clause> const with_clause = "with_clause";
x3::rule<class from_clause> const from_clause = "from_clause";
x3::rule<class fields, FieldsAttr> const fields = "fields";
x3::rule<class field, FieldAttr> const field = "field";
x3::rule<class sql_word> const sql_word = "sql_word";
x3::rule<class sub_expression> const sub_expression = "sub_expression";
x3::rule<class identifier> const identifier = "identifier";
x3::rule<class squoted> const squoted = "squoted";
x3::rule<class dquoted> const dquoted = "dquoted";
x3::rule<class other> const other = "other";
x3::rule<class special> const special = "other";

const auto query_expression_def
  = select_expression % compound_operator
  ;
const auto select_expression_def
  = with_clause
    >> x3::no_case["select"]
    >> - ( distinct_clause
           | x3::no_case["all"]
           )
    >> fields
    >> -(x3::no_case["from"] > from_clause )
  ;
const auto distinct_clause_def
  = x3::no_case["distinct"] >>
    -(x3::no_case["on"] >> '(' >> x3::omit[fields] >> ')')
  ;
const auto compound_operator_def
  = ( x3::no_case["union"] >> -x3::no_case["all"] )
  | x3::no_case["intersect"]
  | x3::no_case["except"]
  ;
const auto with_clause_def
  = *(sql_word - x3::no_case["select"])
  ;
const auto from_clause_def
  = +(sql_word - compound_operator)
  ;
const auto fields_def
  = field % ','
  ;
const auto field_def
  = x3::raw[+(sub_expression | (identifier - x3::lexeme[x3::no_case["from"] >> +x3::ascii::space]))]
  ;
const auto sql_word_def
  = ',' | sub_expression | identifier
  ;
const auto sub_expression_def
  = '(' > *sql_word > ')'
  ;
const auto identifier_def
  = squoted
  | dquoted
  | other;
const auto squoted_def = x3::lexeme[ '\'' > ( *(x3::char_ - '\'') % "''" ) > '\'' ];
const auto dquoted_def = x3::lexeme[ '"' > *(x3::char_ - '"') > '"' ];
const auto other_def = x3::lexeme[ +(x3::graph - special) ];
const auto special_def = x3::char_("()'\",");

BOOST_SPIRIT_DEFINE(query_expression,
                    select_expression,
                    distinct_clause,
                    compound_operator,
                    with_clause,
                    from_clause,
                    fields,
                    field,
                    sql_word,
                    sub_expression,
                    identifier,
                    squoted,
                    dquoted,
                    other,
                    special);

struct error_handler
{
  template<typename Iterator, typename Exception, typename Context>
  x3::error_handler_result on_error(
      Iterator &first, Iterator const &last,
      Exception const &x, Context const &context)
  {
    auto& error_handler = x3::get<x3::error_handler_tag>(context).get();
    std::string message = "Error parsing SQL query: Expected " + x.which() + " here:";
    error_handler(x.where(), message);
    return x3::error_handler_result::fail;
  }
};

struct query_expression_class : error_handler {};

}

void parseSql(const std::string &sql,
              SelectFieldLists &fieldLists)
{
  std::string::const_iterator iter = sql.begin();
  std::string::const_iterator end = sql.end();

  std::ostringstream err_s;
  err_s.imbue(std::locale::classic());

  using error_handler_type = x3::error_handler<std::string::const_iterator>;
  error_handler_type error_handler(iter, end, err_s);

  using sql_parser::query_expression;
  auto const parser =
      x3::with<x3::error_handler_tag>(std::ref(error_handler))
      [
        query_expression
      ];

  sql_parser::QueryExprAttr result;
  bool success = x3::phrase_parse(iter, end, parser, x3::ascii::space, result);

  if (!err_s.str().empty()) {
    LOG_ERROR(err_s.str());
  }

  if (success) {
    if (iter != end) {
      throw Exception("Error parsing SQL query: Expected end here: \""
                      + std::string(iter, end) + "\"");
    } else {
      for (std::size_t i = 0; i < result.size(); ++i) {
        fieldLists.push_back(SelectFieldList());
        SelectFieldList &list = fieldLists.back();
        for (std::size_t j = 0; j < result[i].size(); ++j) {
          list.push_back(SelectField());
          SelectField &field = list.back();
          field.begin = result[i][j].begin() - sql.begin();
          field.end = result[i][j].end() - sql.begin();
        }
      }
    }
  } else
    throw Exception("Error parsing SQL query: \"" + sql + "\"");
}

#endif // X3_QUERY_PARSE

    }
  }
}
