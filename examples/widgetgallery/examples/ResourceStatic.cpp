#include <Wt/WContainerWidget.h>
#include <Wt/WPainter.h>
#include <Wt/WResource.h>
#include <Wt/WServer.h>

SAMPLE_BEGIN(ResourceStatic)

auto container = std::make_unique<Wt::WContainerWidget>();

//auto resource = std::make_shared<SamplePdfResource>();

//WServer::addResource(resource, "/media/static-resource");

SAMPLE_END(return std::move(container))
