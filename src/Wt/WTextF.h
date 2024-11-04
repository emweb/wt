// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2026 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WTEXTF_H_
#define WTEXTF_H_

#include <Wt/WDllDefs.h>
#include <Wt/WJavaScriptExposableObject.h>
#include <Wt/WString.h>

namespace Wt {

/*! \class WTextF Wt/WTextF.h Wt/WTextF.h
 *  \brief A value class that defines a text.
 *
 * The text is defined by a string.
 *
 * <h3>JavaScript exposability</h3>
 *
 * A %WTextF is JavaScript exposable. If a %WTextF \link isJavaScriptBound()
 * is JavaScript bound\endlink, it can be accessed in your custom JavaScript
 * code through \link WJavaScriptHandle::jsRef() its handle's jsRef()\endlink.
 * A text is represented in JavaScript as a string.
 *
 * \warning A %WTextF that is JavaScript exposed should be modified only through its \link WJavaScriptHandle handle\endlink.
 *          Any attempt at modifying it will cause an exception to be thrown.
 *
 * \sa WPaintedWidget::createJSText()
 *
 * \ingroup painting
 */
class WT_API WTextF : public WJavaScriptExposableObject
{
public:

  /*! \brief Default constructor.
   *
   * Constructs an empty text.
   *
   * \sa empty()
   */
  WTextF();

  /*! \brief Construct text from WString.
   */
  WTextF(const WString &text);

  /*! \brief Construct text from string.
   */
  WTextF(const char *text);

  /*! \brief Construct text from string.
   */
  WTextF(const std::string &text);

  WTextF(const WTextF &other);

  /*! \brief Internal assign method.
   */
  WTextF& operator=(const WTextF& other);

  /*! \brief Comparison operator.
   */
  bool operator== (const WTextF& other) const;
  bool operator!= (const WTextF& other) const;

  /*! \brief Returns whether the text is empty or not.
   */
  bool empty() const { return text_.empty(); }

  /*! \brief Sets the content of the text.
   *
   * \throws WException if the text \link isJavaScriptBound() is JavaScript bound\endlink
   */
  void setText(const WString &text);

  /*! \brief Returns the content of the text.
   */
  WString text() const { return text_; }

  std::string jsValue() const override;

  /*! \brief Convert WTextF to WString.
   *
   * \throws WException if the text \link isJavaScriptBound() is JavaScript bound\endlink
   */
  WT_DEPRECATED("Wt now uses WTextF instead of WString in WPaintDevice and WPainter. Use WTextF.text() to get the WString.")
  operator WString() const;

protected:
  void assignFromJSON(const Json::Value &value) override;

private:
  WString text_;
};

}

#endif // WTEXTF_H_
