#include "EntryPoint.h"

#include <algorithm>

namespace Wt {

EntryPoint::EntryPoint(EntryPointType type,
                       ApplicationCreator appCallback,
                       const std::string& path,
                       const std::string& favicon)
  : type_(type),
    resource_(nullptr),
    appCallback_(std::move(appCallback)),
    path_(path),
    favicon_(favicon)
{ }

EntryPoint::EntryPoint(WResource *resource,
                       const std::string& path)
  : type_(EntryPointType::StaticResource),
    resource_(resource),
    appCallback_(nullptr),
    path_(path)
{ }

EntryPoint::EntryPoint(const std::shared_ptr<WResource>& resource, const std::string& path)
  : type_(EntryPointType::StaticResource),
    resource_(resource.get()),
    ownedResource_(resource),
    appCallback_(nullptr),
    path_(path)
{ }

EntryPoint::~EntryPoint() = default;

void EntryPoint::setPath(const std::string& path)
{
  path_ = path;
}

bool EntryPointMatch::operator<(const EntryPointMatch &other) const noexcept
{
  // A successful match is preferred over an unsuccessful one
  if (!entryPoint && !other.entryPoint) {
    // Both no entrypoint, so equivalent
    return false;
  } else if (entryPoint && !other.entryPoint) {
    // This one comes before the other, because this one has an entrypoint
    return true;
  } else if (!entryPoint && other.entryPoint) {
    // This one comes after the other, because this one does not have an entrypoint
    return false;
  }
  assert(entryPoint && other.entryPoint);

  // Prefer the most static match
  const auto& path1 = entryPoint->path();
  const auto& path2 = other.entryPoint->path();
  const auto isSlashOrDollar = [](char c) { return c == '/' || c == '$'; };
  auto sodIt1 = std::find_if(begin(path1), end(path1), isSlashOrDollar);
  auto sodIt2 = std::find_if(begin(path2), end(path2), isSlashOrDollar);
  while (sodIt1 != end(path1) && sodIt2 != end(path2)) {
    if (*sodIt1 == '/' && *sodIt2 == '$') {
      // Encountered dynamic part in other, prefer this one
      return true;
    }
    if (*sodIt1 == '$' && *sodIt2 == '/') {
      // Encountered dynamic part in this, prefer other
      return false;
    }
    sodIt1 = std::find_if(sodIt1 + 1, end(path1), isSlashOrDollar);
    sodIt2 = std::find_if(sodIt2 + 1, end(path2), isSlashOrDollar);
  }

  // Prefer the deepest match
  return depth() > other.depth();
}

std::size_t EntryPointMatch::depth() const noexcept
{
  if (!entryPoint) {
    return 0;
  }

  const auto& path = entryPoint->path();
  return std::count(begin(path), end(path), '/');
}

}
