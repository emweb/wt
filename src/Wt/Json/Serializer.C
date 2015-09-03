#include "Wt/Json/Serializer"

#include "Wt/Json/Object"
#include "Wt/Json/Array"
#include "Wt/Json/Value"
#include "Wt/WWebWidget"
#include "EscapeOStream.h"
#include "WebUtils.h"

#include <set>
#include "boost/lexical_cast.hpp"
#include "boost/algorithm/string/replace.hpp"

namespace Wt {
  namespace Json {

void appendEscaped(const std::string& val, EscapeOStream& result)
{
  result << "\"";
  result.pushEscape(EscapeOStream::JsStringLiteralDQuote);
  result << val;
  result.popEscape();
  result << "\"";
}

void serialize(const Value& val, int indentation, EscapeOStream &result)
{
  char buf[30];
  switch (val.type()) {
  case NullType:
    result << ("null");
    break;
  case StringType:
	appendEscaped(val, result);
	return;
    break;
  case BoolType:
    if ((bool)val)
      result << ("true");
    else
      result << ("false");
	return;
    break;
  case NumberType: 
    {
      double intpart;
      if (fabs(std::modf(val, &intpart)) == 0.0 && fabs(intpart) < 9.22E18)
	result << (long long)intpart;
      else 
	result << Utils::round_js_str(static_cast<double>(val), 16, buf);
      return;
    }
    break;
  case ObjectType:
    serialize((const Object&)val, indentation + 1, result);
	return;
    break;
  case ArrayType:
    serialize((const Array&)val, indentation + 1, result);
	return;
    break;
  }
}


std::string serialize(const Object& obj, int indentation)
{
  EscapeOStream result;
  serialize(obj, indentation, result);
  return result.str();
}

void serialize(const Object& obj, int indentation, EscapeOStream& result)
{
  result << ("{\n");

  for( Object::const_iterator it = obj.begin(); it != obj.end(); ++it) {
    
	// indent values
    for (int i=0; i<indentation; ++i)
      result << ("\t");

    // key (= string-type)
	appendEscaped(it->first, result);

    // name-separator
    result << (" : ");

    // value
    const Value& val = obj.get(it->first);
	serialize(val, indentation, result);

    // value-separator
    if (it != --obj.end())
      result << (",\n");
    else
      result << ("\n");
  }

  for (int i=0; i<indentation-1; ++i)
      result << ("\t");
  result << ("}");

}

std::string serialize(const Array& arr, int indentation)
{
  EscapeOStream result;
  serialize(arr, indentation, result);
  return result.str();
}

void serialize(const Array& arr, int indentation, EscapeOStream& result)
{
  result << ("[\n");

  for (unsigned i = 0; i < arr.size(); ++i) {
	// indent values
	for (int j = 0; j < indentation; ++j)
	  result << ("\t");

	// value
	const Value& val = arr[i];
	serialize(val, indentation, result);

	// value-separator
	if (i < arr.size() - 1)
	  result << (",\n");
	else
	  result << ("\n");
  }

  for (int i = 0; i < indentation - 1; ++i)
	result << ("\t");

  result << ("]");
}
}
}
