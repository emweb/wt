#include "WInvalidFormatException.h"

namespace Wt {
WInvalidFormatException::WInvalidFormatException(const std::string& what)
  : WException(what)
{
}

WInvalidFormatException::WInvalidFormatException(const std::string& what, const std::exception& wrapped)
  : WException(what, wrapped)
{
}
}
