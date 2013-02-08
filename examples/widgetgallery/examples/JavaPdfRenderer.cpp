#include <Wt/WPushButton>
#include <Wt/WResource>

class ReportResource : public Wt::WResource
{
public:
  ReportResource(Wt::WObject *parent = 0);
};

SAMPLE_BEGIN(PdfRenderer)
Wt::WContainerWidget *container = new Wt::WContainerWidget();

Wt::WText *text = new Wt::WText(Wt::WString::tr("report.example"), container);
text->setStyleClass("reset");

Wt::WPushButton *button = new Wt::WPushButton("Create pdf", container);

Wt::WResource *pdf = new ReportResource(container);
button->setLink(pdf);

SAMPLE_END(return container)
