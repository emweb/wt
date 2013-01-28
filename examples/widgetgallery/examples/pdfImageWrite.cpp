#include <Wt/Chart/WCartesianChart>
#include <Wt/WPdfImage>

Wt::Chart::WCartesianChart *chart = new Wt::Chart::WCartesianChart(...);

Wt::WPdfImage pdfImage("4cm", "3cm");
{
    Wt::WPainter p(&pdfImage);
    chart->paint(p);
}
std::ofstream f("chart.pdf", std::ios::out | std::ios::binary);
pdfImage.write(f);
