#include "EntryPoint.h"

namespace Wt {

EntryPoint::EntryPoint(EntryPointType type, ApplicationCreator appCallback,
		       const std::string& path, const std::string& favicon)
  : type_(type),
    resource_(0),
    appCallback_(appCallback),
    path_(path),
    favicon_(favicon)
{ }

EntryPoint::EntryPoint(WResource *resource, const std::string& path)
  : type_(EntryPointType::StaticResource),
    resource_(resource),
    appCallback_(nullptr),
    path_(path)
{ }

EntryPoint::~EntryPoint()
{
}

void EntryPoint::setPath(const std::string& path)
{
  path_ = path;
}

}
