#include "Wt/WQrCode.h"

#include <Wt/WBrush.h>
#include <Wt/WPaintedWidget.h>
#include <Wt/WPainter.h>

#include <string>
#include <exception>
#include <thirdparty/qrcodegen/QrCode.hpp>

namespace Wt {

LOGGER("WQrCode");

WQrCode::WQrCode()
  : WPaintedWidget(),
    errCorrLvl_(ErrorCorrectionLevel::LOW),
    msg_(""),
    code_(),
    squareSize_(5.0)
{
  init();
}

WQrCode::WQrCode(const std::string& message, double squareSize)
  : WPaintedWidget(),
    errCorrLvl_(ErrorCorrectionLevel::LOW),
    msg_(message),
    code_(),
    squareSize_(squareSize)
{
  init();
}

WQrCode::WQrCode(const std::string& message, ErrorCorrectionLevel ecl, double squareSize)
  : WPaintedWidget(),
    errCorrLvl_(ecl),
    msg_(message),
    code_(),
    squareSize_(squareSize)
{
  init();
}

void WQrCode::setErrorCorrectionLevel(ErrorCorrectionLevel ecl)
{
  if (errCorrLvl_ != ecl) {
    errCorrLvl_ = ecl;
    generateCode();
  }
}

void WQrCode::setMessage(const std::string& message)
{
  if (msg_ != message) {
    msg_ = message;
    generateCode();
  }
}

void WQrCode::setSquareSize(double size)
{
  if (squareSize_ != size) {
    squareSize_ = size;
    updateSize();
  }
}

void WQrCode::paintEvent(WPaintDevice* paintDevice)
{
  WPainter painter(paintDevice);
  auto brush = WBrush(StandardColor::Black);

  if (code()) {
    for (auto line = 0; line < code()->getSize(); ++line) {
      for (auto column = 0; column < code()->getSize(); ++column) {
        if (code()->getModule(column, line)) {
          painter.fillRect(line * squareSize(), column * squareSize(),
                            squareSize(), squareSize(),
                            brush);
        }
      }
    }
  }
}

void WQrCode::init()
{
  generateCode();
}

void WQrCode::generateCode()
{
  try {
#ifndef WT_TARGET_JAVA
    code_ = std::make_unique<qrcodegen::QrCode>(qrcodegen::QrCode::encodeText(msg_.c_str(), errCorrLvl_));
#else
    qrcodegen::QrCode code = qrcodegen::QrCode::encodeText(msg_.c_str(), errCorrLvl_);
    code_ = &code;
#endif
  } catch (std::exception& e) {
    code_.reset();
    LOG_ERROR("Error while generating QR code: "<<e.what());
  }
  updateSize();
}

void WQrCode::updateSize()
{
  auto size = code() ? code()->getSize() * squareSize_ : 0;
  resize(size, size);
  update();
}

}

