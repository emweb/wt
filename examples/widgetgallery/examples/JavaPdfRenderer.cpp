#include <Wt/WPushButton.h>
#include <Wt/WResource.h>

class ReportResource : public Wt::WResource
{
public:
  ReportResource();
};

SAMPLE_BEGIN(PdfRenderer)

auto container = std::make_unique<Wt::WContainerWidget>();

Wt::WText *text =
    container->addNew<Wt::WText>(Wt::WString::tr("report.example"));
text->setStyleClass("reset");

Wt::WPushButton *button =
    container->addNew<Wt::WPushButton>("Create pdf");

auto pdf = std::make_shared<ReportResource>();
button->setLink(Wt::WLink(pdf));

SAMPLE_END(return container)
