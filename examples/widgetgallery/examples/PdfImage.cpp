#include <Wt/WPainter>
#include <Wt/WPdfImage>
#include <Wt/WPushButton>
#include <Wt/WResource>

class SamplePdfResource : public Wt::WPdfImage
{
public:
    SamplePdfResource(Wt::WObject *parent = 0)
        : Wt::WPdfImage(400, 300, parent)
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

        painter.drawText(0, 0, 400, 300, Wt::AlignCenter | Wt::AlignTop,
                         "Hello, PDF");
    }
};

SAMPLE_BEGIN(PdfImage)
Wt::WContainerWidget *container = new Wt::WContainerWidget();

Wt::WResource *pdf = new SamplePdfResource(container);

Wt::WPushButton *button = new Wt::WPushButton("Create pdf", container);
button->setLink(pdf);

SAMPLE_END(return container)
