/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Query"
#include "Query_impl.h"
#include "Exception"
#include "ptr"

#include <boost/version.hpp>

#if !defined(WT_NO_SPIRIT) && BOOST_VERSION >= 104100
#  define SPIRIT_QUERY_PARSE
#endif

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
#include <boost/bind.hpp>
#include <iostream>

#else
#include <boost/range/iterator_range.hpp>
#include <boost/algorithm/string.hpp>

#endif // SPIRIT_QUERY_PARSE

namespace Wt {
  namespace Dbo {
    namespace Impl {

#ifndef SPIRIT_QUERY_PARSE
void parseSql(const std::string& sql, SelectFieldLists& fieldLists,
	      bool& simpleSelectCount)
{
  fieldLists.clear();
  simpleSelectCount = true;

  std::string::const_iterator i = sql.begin();

  std::size_t selectPos = ifind(sql, "select ");
  if (selectPos != 0)
    throw Exception("Session::query(): query should start with 'select '"
		    " (sql='" + sql + "')");

  i += 7;

  std::size_t distinctPos = ifind(sql.substr(i - sql.begin()), "distinct ");
  if (distinctPos == 0) {
    simpleSelectCount = false;
    i += 9;
  } else {
    std::size_t allPos = ifind(sql.substr(i - sql.begin()), "all ");
    if (allPos == 0) {
      simpleSelectCount = false;
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

#else // SPIRIT_QUERY_PARSE

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

  sql_query_grammar(Iterator parseBegin, SelectFieldLists& fieldLists,
		    bool& simpleSelectCount)
    : sql_query_grammar::base_type(query_expression),
      parseBegin_(parseBegin),
      fieldLists_(fieldLists),
      simpleSelectCount_(simpleSelectCount)
  {
    using qi::lit;
    using qi::lexeme;
    using qi::no_case;
    using qi::raw;
    using qi::on_error;
    using qi::fail;
    using ascii::char_;
    using ascii::graph;

    using phoenix::construct;
    using phoenix::val;
    using phoenix::throw_;

    query_expression 
      = select_expression % compound_operator[
					      boost::bind(&Self::notSimple,this)
					      ]
      ;

    select_expression 
      = with_clause
        >> no_case["select"][
			     boost::bind(&Self::handleSelect, this)
			     ]
	>> -( (no_case["distinct"] >>
	      -(no_case["on"] >> '(' >> (raw[field] % ',') >>  ')'))[
				  boost::bind(&Self::notSimple,this)
				  ]
	      | no_case["all"][
			       boost::bind(&Self::notSimple,this)
			       ]
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
      = *(sql_word[
		   boost::bind(&Self::notSimple, this)
		   ] - no_case["select"])
      ;

    from_clause
      = +(sql_word - compound_operator)
      ;

    fields
      = raw[field][
		   boost::bind(&Self::handleField, this, _1)
		   ] 
        % ','
      ;

    field
      = *(sql_word - (no_case["from"] | lit(',')))
      ;

    sql_word
      = ',' | identifier | sub_expression
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
       std::cerr 
       << val("Error parsing SQL query: Expected ")
       << boost::spirit::_4
       << val(" here: \"")
       << construct<std::string>(boost::spirit::_3, boost::spirit::_2)
       << val("\"")
       << std::endl);

    simpleSelectCount_ = true;
  }

  Iterator parseBegin_;
  SelectFieldLists& fieldLists_;
  bool& simpleSelectCount_;

  qi::rule<Iterator, ascii::space_type> query_expression,
    select_expression, compound_operator, with_clause, from_clause,
    fields, field, sql_word, sub_expression, identifier, squoted, dquoted,
    other;

  qi::rule<Iterator> special;

  void notSimple()
  {
    simpleSelectCount_ = false;
  }

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

void parseSql(const std::string& sql, SelectFieldLists& fieldLists,
	      bool& simpleSelectCount)
{
  std::string::const_iterator iter = sql.begin();
  std::string::const_iterator end = sql.end();

  sql_query_grammar<std::string::const_iterator>
    sql_grammar(iter, fieldLists, simpleSelectCount);

  bool success = qi::phrase_parse(iter, end, sql_grammar, ascii::space);

  if (success) {
    if (iter != end)
      throw Exception("Error parsing SQL query: Expected end here:\""
		      + std::string(iter, end) + "\"");
  } else
    throw Exception("Error parsing SQL query: \"" + sql + "\"");
}

#endif // SPIRIT_QUERY_PARSE

    }
  }
}
				 
