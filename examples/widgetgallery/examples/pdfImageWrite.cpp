#include <Wt/Chart/WCartesianChart.h>
#include <Wt/WPdfImage.h>

auto chart = std::make_unique<Chart::WCartesianChart>();

WPdfImage pdfImage("4cm", "3cm");
{
    WPainter p(&pdfImage);
    chart->paint(p);
}
std::ofstream f("chart.pdf", std::ios::out | std::ios::binary);
pdfImage.write(f);
