#include <Wt/WPainter.h>
#include <Wt/WPdfImage.h>
#include <Wt/WPushButton.h>
#include <Wt/WResource.h>

class SamplePdfResource : public Wt::WPdfImage
{
public:
    SamplePdfResource()
        : WPdfImage(400, 300)
    {
        suggestFileName("line.pdf");
        paint();
    }

private:
    void paint() {
        Wt::WPainter painter(this);

        Wt::WPen thickPen;
        thickPen.setWidth(5);
        painter.setPen(thickPen);
        painter.drawLine(50, 250, 150, 50);
        painter.drawLine(150, 50, 250, 50);

        painter.drawText(0, 0, 400, 300,
                         Wt::AlignmentFlag::Center | Wt::AlignmentFlag::Top,
                         "Hello, PDF");
    }
};

SAMPLE_BEGIN(PdfImage)
auto container = Wt::cpp14::make_unique<Wt::WContainerWidget>();

auto pdf = std::make_shared<SamplePdfResource>();

Wt::WPushButton *button =
    container->addWidget(Wt::cpp14::make_unique<Wt::WPushButton>("Create pdf"));
button->setLink(Wt::WLink(pdf));
SAMPLE_END(return std::move(container))
