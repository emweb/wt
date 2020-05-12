// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2020 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_WCOLORPICKER_H_
#define WT_WCOLORPICKER_H_

#include <Wt/WColor.h>
#include <Wt/WDllDefs.h>
#include <Wt/WFormWidget.h>
#include <Wt/WObject.h>
#include <Wt/WSignal.h>
#include <Wt/WWebWidget.h>

namespace Wt {

/*! \class WColorPicker Wt/WColorPicker.h Wt/WColorPicker.h
 *  \brief A widget that provides a browser-native color picker.
 *
 * To act upon color value changes, connect a slot to the changed()
 * signal. This signal is emitted when the user changes the selected color,
 * and subsequently closes the color picker.
 *
 * To act upon any color change, connect a slot to the colorInput() signal.
 * Note that this signal may fire very quickly depending on how the browser's
 * color picker works.
 *
 * At all times, the currently selected color may be accessed with the value() method.
 *
 * The widget corresponds to the HTML <tt>&lt;input type="color"&gt;</tt> tag.
 * Note that this element does not support CSS color names.  When manipulating
 * this widget with \link WColor WColor\endlink values, ensure they have
 * valid RGB values or the color picker will reset to #000000.
 *
 * %WColorPicker is an \link WWidget::setInline(bool) inline \endlink widget.
 *
 * \sa WColor
 */
class WT_API WColorPicker : public WFormWidget
{
public:
  /*! \brief Creates a color picker with the default color of black (#000000).
   */
  WColorPicker();

  /*! \brief Creates a color picker with the given color value.
   * Ensure the color has valid RGB values, or the color will be reset to #000000.
   *
   * \sa WColor::WColor(const WString&)
   */
  WColorPicker(const WColor& color);

  /*! \brief Returns the current value of the color picker
   * as a \link WColor WColor\endlink object.
   *
   * \sa setColor(const WColor&)
   */
  WColor color() const;

  /*! \brief Sets the selected color.
   *
   * The default value is #000000 (black).
   *
   * Ensure the color has valid RGB values, or the color will be reset to #000000.
   *
   * \sa color()
   * \sa WColor::WColor(const WString&)
   */
  void setColor(const WColor& value);

  /*! \brief Event signal emitted when the selected color is changed.
   *
   * This signal is emitted whenever the selected color has
   * changed. Unlike the changed() signal, this signal is fired on
   * every change, not only when the color picker is closed.
   *
   * In particular, on browsers with a draggable color picker (i.e. most common browsers),
   * this signal fires every time the position changes.  Use with caution.
   *
   * \sa changed()
   */
  EventSignal<>& colorInput();

  /*! \brief Returns the current value of the color picker as a string.
   *
   * This is implemented as
   * \code
   * return color().cssText();
   * \endcode
   */
  virtual WT_USTRING valueText() const override;

  /*! \brief Sets the current value of the color picker as a string.
   * The string must be in a format from which \link WColor WColor\endlink
   * can determine RGB values (i.e. not a CSS color name),
   * or the value will be set to #000000.
   *
   * This is implemented as
   * \code
   * setColor(WColor(value));
   * \endcode
   *
   * \sa WColor::WColor(const WString&)
   */
  virtual void setValueText(const WT_USTRING& value) override;

private:
  static const char *INPUT_SIGNAL;
  WColor color_;
  bool colorChanged_;

protected:
  virtual void updateDom(DomElement& element, bool all) override;
  virtual DomElementType domElementType() const override;
  virtual void propagateRenderOk(bool deep) override;
  virtual void setFormData(const FormData& formData) override;
};

}

#endif  // WT_WCOLORPICKER_H_
