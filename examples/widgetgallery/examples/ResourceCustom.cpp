#include <Wt/WAnchor.h>
#include <Wt/WContainerWidget.h>
#include <Wt/Http/Request.h>
#include <Wt/Http/Response.h>
#include <Wt/WObject.h>
#include <Wt/WResource.h>

class MyResource : public Wt::WResource
{
public:
    MyResource()
        : WResource()
    {
	suggestFileName("data.txt");
    }

    ~MyResource() {
	beingDeleted();
    }

    void handleRequest(const Wt::Http::Request &request,
                       Wt::Http::Response &response) {
	response.setMimeType("plain/text");
	response.out() << "I am a text file." << std::endl;
    }
};

SAMPLE_BEGIN(ResourceCustom)
auto container = std::make_unique<Wt::WContainerWidget>();

auto textResource = std::make_shared<MyResource>();

Wt::WLink link = Wt::WLink(textResource);
link.setTarget(Wt::LinkTarget::NewWindow);
Wt::WAnchor *anchor = container->addNew<Wt::WAnchor>(link,"Download file");

SAMPLE_END(return std::move(container))

