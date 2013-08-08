#include "Wt/Json/Serializer"

#include "Wt/Json/Object"
#include "Wt/Json/Array"
#include "Wt/Json/Value"

#include <set>
#include "boost/lexical_cast.hpp"

namespace Wt {
  namespace Json {

std::string serialize(const Object& obj, int indentation)
{
  std::string result("{\n");
  std::set< std::string > keys = obj.names();

  for (std::set<std::string>::iterator it = keys.begin(); it != keys.end();
       it++) {
    // indent values
    for (int i=0; i<indentation; i++)
      result.append("\t");

    // key (= string-type)
    result.append("\"").append(*it).append("\"");

    // name-separator
    result.append(" : ");

    // value
    const Value& val = obj.get(*it);
    switch (val.type()) {
    case NullType:
      result.append("null");
      break;
    case StringType:
      result.append("\"").append(val).append("\"");
      break;
    case BoolType:
      if ((bool)val)
	result.append("true");
      else
	result.append("false");
      break;
    case NumberType:
      result.append(val.toString());
      break;
    case ObjectType:
      result.append(serialize((Object)val, indentation+1));
      break;
    case ArrayType:
      result.append(serialize((Array)val, indentation+1));
      break;
    }

    // value-separator
    if (it != --keys.end())
      result.append(",\n");
    else
      result.append("\n");
  }

  for (int i=0; i<indentation-1; i++)
      result.append("\t");
  result.append("}");

  return result;
}

std::string serialize(const Array& arr, int indentation)
{
  std::string result("[\n");

  for (int i=0; i<arr.size(); i++) {
    // indent values
    for (int j=0; j<indentation; j++)
      result.append("\t");

    // value
    const Value& val = arr[i];
    switch (val.type()) {
    case NullType:
      result.append("null");
      break;
    case StringType:
      result.append("\"").append(val).append("\"");
      break;
    case BoolType:
      if ((bool)val)
	result.append("true");
      else
	result.append("false");
      break;
    case NumberType:
      result.append(val.toString());
      break;
    case ObjectType:
      result.append(serialize((Object)val, indentation+1));
      break;
    case ArrayType:
      result.append(serialize((Array)val, indentation+1));
      break;
    }

    // value-separator
    if (i < arr.size()-1)
      result.append(",\n");
    else
      result.append("\n");
  }

  for (int i=0; i<indentation-1; i++)
      result.append("\t");
  result.append("]");

  return result;
}


  }
}
