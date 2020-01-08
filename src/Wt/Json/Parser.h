// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_JSON_PARSER_H_
#define WT_JSON_PARSER_H_

#include <string>
#include <Wt/WException.h>

namespace Wt {
  namespace Json {

class Object;
class Value;
class Array;

/*! \brief A parse error.
 *
 * Exception thrown or returned by a JSON parsing function.
 *
 * \ingroup json
 */
class WT_API ParseError : public WException
{
public:
  ParseError();
  ParseError(const std::string& message);

  void setError(const std::string& message);
};

/*! \brief Parse function
 *
 * This function parses the input string (which represents a UTF-8
 * JSON-encoded data structure) into the \p result value. On success,
 * the result value contains either an Array or Object.
 *
 * If validateUTF8 is true, the parser will sanitize (security scan for
 * invalid UTF-8) the UTF-8 input string before parsing starts.
 *
 * \throws ParseError when the input is not a correct JSON structure.
 *
 * \ingroup json
 */
WT_API extern void parse(const std::string& input, Value& result, bool validateUTF8 = true);

/*! \brief Parse function
 *
 * This function parses the input string (which represents a UTF-8
 * JSON-encoded data structure) into the \p result value. On success,
 * the result value contains either an Array or Object.
 *
 * If validateUTF8 is true, the parser will sanitize (security scan for
 * invalid UTF-8) the UTF-8 input string before parsing starts.
 *
 * This method returns \c true if the parse was succesful, or reports an
 * error in into the \p error value otherwise.
 *
 * \ingroup json
 */
WT_API extern bool parse(const std::string& input, Value& result,
                         ParseError& error, bool validateUTF8 = true);

/*! \brief Parse function
 *
 * This function parses the input string (which represents a UTF-8
 * JSON-encoded data structure) into the \p result object.
 *
 * If validateUTF8 is true, the parser will sanitize (security scan for
 * invalid UTF-8) the UTF-8 input string before parsing starts.
 *
 * \throws ParseError when the input is not a correct JSON structure.
 * \throws TypeException when the JSON structure does not represent an Object.
 *
 * \ingroup json
 */
WT_API extern void parse(const std::string& input, Object& result,
                         bool validateUTF8 = true);

/*! \brief Parse function
 *
 * This function parses the input string (which represents a UTF-8
 * JSON-encoded data structure) into the \p result value. On success,
 * the result value contains either an Array or Object.
 *
 * If validateUTF8 is true, the parser will sanitize (security scan for
 * invalid UTF-8) the UTF-8 input string before parsing starts.
 *
 * This method returns \c true if the parse was succesful, or reports an
 * error in into the \p error value otherwise.
 *
 * \ingroup json
 */
WT_API extern bool parse(const std::string& input, Object& result,
                         ParseError& error, bool validateUTF8 = true);

/*! \brief Parse function
 *
 * This function parses the input string (which represents a UTF-8
 * JSON-encoded data structure) into the \p result object.
 *
 * If validateUTF8 is true, the parser will sanitize (security scan for
 * invalid UTF-8) the UTF-8 input string before parsing starts.
 *
 * \throws ParseError when the input is not a correct JSON structure.
 * \throws TypeException when the JSON structure does not represent an Object.
 *
 * \ingroup json
 */
WT_API extern void parse(const std::string& input, Array& result,
                         bool validateUTF8 = true);

/*! \brief Parse function
 *
 * This function parses the input string (which represents a UTF-8
 * JSON-encoded data structure) into the \p result value. On success,
 * the result value contains either an Array or Object.
 *
 * If validateUTF8 is true, the parser will sanitize (security scan for
 * invalid UTF-8) the UTF-8 input string before parsing starts.
 *
 * This method returns \c true if the parse was succesful, or reports an
 * error in into the \p error value otherwise.
 *
 * \ingroup json
 */
WT_API extern bool parse(const std::string& input, Array& result,
                         ParseError& error, bool validateUTF8 = true);

#ifdef WT_TARGET_JAVA
    class Parser {
      Object parse(const std::string& input, bool validateUTF8 = true);
    };
#endif

  }
}

#endif // WT_JSON_PARSER_H_
