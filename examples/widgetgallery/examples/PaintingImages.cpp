#include <Wt/WBrush.h>
#include <Wt/WColor.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WPaintDevice.h>
#include <Wt/WPaintedWidget.h>
#include <Wt/WPainter.h>

class PaintingImagesWidget : public Wt::WPaintedWidget
{
public:
    PaintingImagesWidget()
        : WPaintedWidget()
    {
	resize(639, 1310);  // Provide a default size.
    }

protected:
    void paintEvent(Wt::WPaintDevice *paintDevice) {
        Wt::WPainter painter(paintDevice);

	Wt::WPainter::Image image("pics/sintel_trailer.jpg", 639, 354);
	painter.drawImage(0.0, 0.0, image);
	// Draw a part of the same image starting at (110, 75) and put it at
	// (0, 364).
	painter.drawImage(0.0, 364.0, image, 110.0, 75.0, 130.0, 110.0);
	// Draw the same part using WPointF for the starting point and WRectF for
	// the source rectangle.
	Wt::WPointF location = Wt::WPointF(0.0, 484.0);
	Wt::WRectF sourceRect = Wt::WRectF(110.0, 75.0, 130.0, 110.0);
	painter.drawImage(location, image, sourceRect);
	// Draw the image in a rectangle.
	Wt::WRectF destinationRect = Wt::WRectF(0.0, 604.0, 130.0, 110.0);
	painter.drawImage(destinationRect, image);
	// Draw a part of the image in a rectangle - scaling down
	sourceRect = Wt::WRectF(60.0, 80.0, 220.0, 180.0);
	destinationRect = Wt::WRectF(0.0, 724.0, 130.0, 110.0);
	painter.drawImage(destinationRect, image, sourceRect);
	// Draw a part of the image in a rectangle - scaling up
	sourceRect = Wt::WRectF(294.0, 226.0, 265.0, 41.0);
	destinationRect = Wt::WRectF(0.0, 844.0, 639.0, 110.0);
	painter.drawImage(destinationRect, image, sourceRect);

	painter.translate(0,964);
	// Draw the image and add shapes to it.
	painter.drawImage(0.0, 0.0, image);
	Wt::WPainterPath path;
	path.addEllipse(369, 91, 116, 116);
	path.addRect(294, 226, 265, 41);
	path.moveTo(92,330);
	path.lineTo(66,261);
	path.lineTo(122,176);
	path.lineTo(143,33);
	path.lineTo(164,33);
	path.lineTo(157,88);
	path.lineTo(210,90);
	path.lineTo(263,264);
	path.lineTo(228,330);
	path.lineTo(92,330);
	Wt::WPen pen = Wt::WPen(Wt::WColor(Wt::StandardColor::Red));
	pen.setWidth(3);
	painter.strokePath(path, pen);
    }
};

SAMPLE_BEGIN(PaintingImages)
auto container = std::make_unique<Wt::WContainerWidget>();

container->addNew<PaintingImagesWidget>();

SAMPLE_END(return std::move(container))
