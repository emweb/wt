#include <Wt/WPushButton.h>
#include <Wt/WResource.h>

class ReportResource : public WResource
{
public:
  ReportResource();
};

SAMPLE_BEGIN(PdfRenderer)

auto container = cpp14::make_unique<WContainerWidget>();

WText *text =
    container->addWidget(cpp14::make_unique<WText>(WString::tr("report.example")));
text->setStyleClass("reset");

WPushButton *button =
    container->addWidget(cpp14::make_unique<WPushButton>("Create pdf"));

auto pdf = std::make_shared<ReportResource>();
button->setLink(WLink(pdf));

SAMPLE_END(return container)
