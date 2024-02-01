#include "WInvalidOperationException.h"

namespace Wt {
WInvalidOperationException::WInvalidOperationException(const std::string& what)
  : WException(what)
{
}

WInvalidOperationException::WInvalidOperationException(const std::string& what, const std::exception& wrapped)
  : WException(what, wrapped)
{
}
}
