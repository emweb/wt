// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2024 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_QRCODE_H_
#define WT_QRCODE_H_

#include "Wt/WBrush.h"
#include "Wt/WPaintedWidget.h"

#include "thirdparty/qrcodegen/QrCode.hpp"

namespace Wt {
/*! \class WQrCode Wt/WQrCode.h Wt/WQrCode.h
 *  \brief A widget representing a QR code.
 *
 * This widget allows to generate QR code from a string.
 *
 * If you wish to alter the look of the QR code (to add a logo
 * in the middle for instance), you can override paintEvent.
 * However, you will still need to call the WQrCode::paintEvent
 * to generate the QR code itself.
 *
 *
 * The content that is encoded is taken as-is. If the purpose of your
 * QR code requires special formatting, the formatted string needs to be
 * supplied to the constructor.
 *
 * By default, the generated QR code will use the lowest possible error
 * correction, and a square size of 5 pixels.
 *
 * Usage example:
 * \if cpp
 * \code
 *   auto website = "https://webtoolkit.eu";
 *   auto app = Wt::WApplication::instance();
 *   root->addNew<WQrCode>(website);
 * \endcode
 * \elseif java
 * \code
 *   String website = "https://webtoolkit.eu";
 *   WApplication app = WApplication.getInstance();
 *   app.getRoot().addWidget(new WQrCode(website));
 * \endcode
 * \endif
 */
class WT_API WQrCode : public WPaintedWidget
{
public:

  /*! \brief Represents the different error correction levels for QR codes.
   *
   * The error correction level impacts how much erroneous squares the QR
   * code can contains without becoming unreadable.
   *
   * For most digital use, the default of LOW is sufficient. Higher levels
   * can be considered for printed out media.
   *
   * \sa setErrorCorrectionLevel()
   */
  enum class ErrorCorrectionLevel
  {
    LOW = 0,      //!< About 7% of incorrect squares is tolerated
    MEDIUM = 1,   //!< About 15% of incorrect squares is tolerated
    QUARTILE = 2, //!< About 25% of incorrect squares is tolerated
    HIGH = 3     //!< About 30% of incorrect squares is tolerated
  };

  //! Creates a default QR code with an empty message.
  WQrCode();

  /*! \brief Creates a QR code.
   *
   * Create a QR code with the given message, and square size.
   *
   * \sa setMessage(), setSquareSize()
   */
  WQrCode(const std::string& message, double squareSize_);

  /*! \brief Creates a QR code.
   *
   * Create a QR code with the given message, error correction level
   * and square size.
   *
   * \sa setMessage(), setErrorCorrectionLevel(), setSquareSize()
   */
  WQrCode(const std::string& message, ErrorCorrectionLevel ecl = ErrorCorrectionLevel::LOW, double squareSize_ = 5.0);

  /*! \brief Set the error correction level of the QR code.
   *
   * Increases the amount of information that can be lost before
   * altering the encoded message. A higher level of error correcting
   * code, makes the QR code more robust to changes.
   *
   * Increasing the error correction level also increases the amount
   * of data needed to encode the message, resulting in a visually
   * bigger QR code than one with a lower error correcting level.
   *
   * By default, ErrorCorrectionLevel::LOW is used.
   *
   * \sa setMessage()
   */
  void setErrorCorrectionLevel(ErrorCorrectionLevel ecl);

  /*! \brief Returns the error correction level of the QR code.
   *
   * \sa setErrorCorrectionLevel()
   */
  ErrorCorrectionLevel errorCorrectionLevel() const { return errCorrLvl_; }

  /*! \brief Set the message of the QR code.
   *
   * This sets the message carried by the QR code. There is a
   * limit to the size of the message which depends on many factors.
   * The most important one is the error correction level. Higher
   * error correction level diminish the maximum size the message
   * can have. Roughly speaking when using the highest level of error
   * correction, the maximum number of allowed content is in the area of
   * 1Kb, whereas for the lowest level of error correction, this is
   * around 3Kb.
   *
   * The longer the message is, the more data needs to be encoded, and
   * thus the bigger the resulting QR code becomes.
   *
   * If the message is to long, the QR code will not be generated.
   *
   * \sa setErrorCorrectionLevel() error()
   */
  void setMessage(const std::string& message);

  /*! \brief Returns the message of the QR code.
   *
   * \sa setMessage()
   */
  std::string message() const { return msg_; }

  /*! \brief Set the size of the dots composing the QR code.
   *
   * Sets the size (in pixels) that each dot of the QR code will have.
   * A single square of the QR code can be seen in any corner of the QR
   * code. There a reference visual is always displayed that contains
   * exactly seven dots in width and height.
   *
   * This allows the application to correctly resize the QR code.
   */
  void setSquareSize(double size);

  /*! \brief Returns the size of the squares composing the QR code.
   *
   * \sa setSquareSize()
   */
  double squareSize() const { return squareSize_; }

  /*! \brief Returns whether an error stopped the generation of the QR code.
   *
   * Returns true if the QR code could not be generated due to an error.
   *
   * In case the QR code was not generated, it's size will be set to 0 and
   * the QR code will not be painted.
   */
  bool error() const { return !code(); }

  /*! \brief Sets the brush with which the QR code is painted.
   *
   * This allows for the color of the QR code to be changed.
   */
  void setBrush(const WBrush& brush);

  /*! \brief Returns the brush with which the QR code is painted.
   *
   * \sa setBrush()
   */
  WBrush brush() const { return brush_; }

protected:
  void paintEvent(WPaintDevice* paintDevice) override;

private:
  ErrorCorrectionLevel errCorrLvl_;
  std::string msg_;
  std::unique_ptr<qrcodegen::QrCode> code_;
  double squareSize_;
  WBrush brush_;

  void init();
  const qrcodegen::QrCode* code() const { return code_.get(); }
  void generateCode();
  void updateSize();
  qrcodegen::Ecc ecc() const;
};

}
#endif // WT_QRCODE_H_
