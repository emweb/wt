#include <Wt/WContainerWidget>
#include <Wt/WPainter>
#include <Wt/WResource>
#include <Wt/WServer>

SAMPLE_BEGIN(ResourceStatic)

Wt::WContainerWidget *container = new Wt::WContainerWidget();

//Wt::WResource *resource = new SamplePdfResource(container);

//Wt::WServer::addResource(resource, "/media/static-resource");

SAMPLE_END(return container)
