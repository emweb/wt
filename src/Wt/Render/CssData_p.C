#include "CssData_p.h"

using namespace Wt::Render;

Term DeclarationBlockImpl::value(const std::string& property) const
{
  std::map<std::string, Term >::const_iterator iter
      = properties_.find(property);
  return iter != properties_.end() ? iter->second : Term();
}
