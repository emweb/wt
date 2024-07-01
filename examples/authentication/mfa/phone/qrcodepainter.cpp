#include "qrcodepainter.h"

QrCodePainter::QrCodePainter(const std::string& url)
  : code_(qrcodegen::QrCode::encodeText(url.c_str(), ErrorLevelCorrection))
{
  auto size = code().getSize() * SQUARE_SIZE;
  resize(size, size);
}

void QrCodePainter::paintEvent(Wt::WPaintDevice* paintDevice)
{
  Wt::WPainter painter(paintDevice);
  auto brush = Wt::WBrush(Wt::StandardColor::Black);

  for (auto line = 0; line < code().getSize(); ++line) {
    for (auto column = 0; column < code().getSize(); ++column) {
      if (code().getModule(column, line)) {
        painter.fillRect(line * SQUARE_SIZE, column * SQUARE_SIZE, SQUARE_SIZE, SQUARE_SIZE, brush);
      }
    }
  }
}

