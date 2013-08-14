#include "Wt/Json/Serializer"

#include "Wt/Json/Object"
#include "Wt/Json/Array"
#include "Wt/Json/Value"
#include "Wt/WWebWidget"

#include <set>
#include "boost/lexical_cast.hpp"

namespace Wt {
  namespace Json {

std::string serialize(const Value& val, int indentation)
{
  switch (val.type()) {
  case NullType:
    return "null";
    break;
  case StringType:
    return WWebWidget::jsStringLiteral((std::string)val, '\"');
    break;
  case BoolType:
    if ((bool)val)
      return "true";
    else
      return "false";
    break;
  case NumberType:
    return val.toString();
    break;
  case ObjectType:
    return serialize((Object)val, indentation+1);
    break;
  case ArrayType:
    return serialize((Array)val, indentation+1);
    break;
  }

  return std::string();
}

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
    result.append(WWebWidget::jsStringLiteral(*it, '\"'));

    // name-separator
    result.append(" : ");

    // value
    const Value& val = obj.get(*it);
    result.append(serialize(val, indentation));

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

  for (unsigned i = 0; i < arr.size(); i++) {
    // indent values
    for (int j = 0; j < indentation; j++)
      result.append("\t");

    // value
    const Value& val = arr[i];
    result.append(serialize(val, indentation));

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
