#include <Wt/WAnchor>
#include <Wt/WContainerWidget>
#include <Wt/Http/Request>
#include <Wt/Http/Response>
#include <Wt/WObject>
#include <Wt/WResource>

class MyResource : public Wt::WResource
{
public:
    MyResource(Wt::WObject *parent = 0)
	: Wt::WResource(parent)
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
Wt::WContainerWidget *container = new Wt::WContainerWidget();

Wt::WResource *textResource = new MyResource(container);

Wt::WAnchor *anchor = new Wt::WAnchor(Wt::WLink(textResource), "Download file", container);
anchor->setTarget(Wt::TargetNewWindow);

SAMPLE_END(return container)

