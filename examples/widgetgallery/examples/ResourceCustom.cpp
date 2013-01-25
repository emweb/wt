#include <Wt/WContainerWidget>
#include <Wt/WObject>
#include <Wt/WResource>

//class MyResource : public Wt::WResource
//{
//public:
//  MyResource(Wt::WObject *parent = 0)
//    : Wt::WResource(parent)
//  {
//    suggestFileName("data.txt");
//  }

//  ~MyResource() {
//    beingDeleted(); // See "Concurrency issues".
//  }

//  void handleRequest(const Wt::Http::Request &request,
//                            Wt::Http::Response &response) {
//    response.setMimeType("plain/text");
//    response.out() << "I am a text file." << std::endl;
//  }
//};

SAMPLE_BEGIN(ResourceCustom)

Wt::WContainerWidget *container = new Wt::WContainerWidget();

//MyResource *resource = new MyResource(container);

SAMPLE_END(return container)

