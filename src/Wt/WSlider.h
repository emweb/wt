// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WSLIDER_H_
#define WSLIDER_H_

#include <Wt/WFormWidget.h>
#include <Wt/WJavaScript.h>

namespace Wt {

class PaintedSlider;

/*! \class WSlider Wt/WSlider.h Wt/WSlider.h
 *  \brief A horizontal or vertical slider control.
 *
 * A slider allows the user to specify an integer value within a particular
 * range using a visual slider.
 *
 * The slider must be sized explicitly using resize() or by a layout
 * manager. The default size is 150 x 50 pixels for a horizontal
 * slider, and 50 x 150 pixels for a vertical slider.
 *
 * \if cpp
 * Usage example:
 * \code
 * auto scaleSlider = std::make_unique<Wt::WSlider>(Wt::Orientation::Horizontal);
 * scaleSlider->setMinimum(0);
 * scaleSlider->setMaximum(20);
 * scaleSlider->setValue(10);
 * scaleSlider->setTickInterval(5);
 * scaleSlider->setTickPosition(Wt::WSlider::TicksBothSides);
 * scaleSlider->resize(300, 50);
 * scaleSlider->valueChanged().connect(this, &ThisClass::scaleShape);
 * \endcode
 * \endif
 *
 * \image html WSlider-1.png "Horizontal slider with ticks on both sides."
 *
 * <h3>CSS</h3>
 *
 * The non-native slider (HTML4, see setNativeControl()) is styled by
 * the current CSS theme.
 */
class WT_API WSlider : public WFormWidget
{
public:
  /*! \brief Enumeration that specifies the location of ticks.
   */
  enum class TickPosition {
    TicksAbove = 0x1,     //!< %Render ticks above (horizontal slider)
    TicksBelow = 0x2     //!< %Render ticks below (horizontal slider)
#ifndef WT_TARGET_JAVA
    ,TicksLeft = 0x1,      //!< %Render ticks on the left side (vertical slider)
    TicksRight = 0x2      //!< %Render ticks on the right side (vertical slider)
#endif // WT_TARGET_JAVA
  };

  //! Do not render ticks.
  static const Wt::WFlags<TickPosition> NoTicks;

  //! %Render ticks on both sides.
  static const Wt::WFlags<TickPosition> TicksBothSides;

  /*! \brief Creates a default horizontal slider.
   *
   * The slider shows no ticks, has a range from 0 to 99, and has tickInterval
   * of 0 (defaulting to three ticks over the whole range).
   *
   * The initial value is 0.
   */
  WSlider();

  /*! \brief Creates a default slider of the given orientation.
   *
   * The slider shows no ticks, has a range from 0 to 99, and has tickInterval
   * of 0 (defaulting to three ticks over the whole range).
   *
   * The initial value is 0.
   */
  WSlider(Orientation orientation);

  /*! \brief Destructor.
   */
  ~WSlider();

  /*! \brief Configures whether a native HTML5 control should be used.
   *
   * When \p native, the new "range" input element, specified by HTML5
   * and when implemented by the browser, is used rather than the
   * built-in element. A native control is styled by the browser
   * (usually in sync with the OS) rather than through the theme
   * chosen. Settings like tick interval and tick position are
   * ignored.
   *
   * \note Vertically oriented sliders are in theory supported by the HTML5
   *       input element, but in practice are usually not rendered correctly
   *       by the browser.
   */
  void setNativeControl(bool nativeControl);

  /*! \brief Returns whether a native HTML5 control is used.
   *
   * Taking into account the preference for a native control,
   * configured using setNativeControl(), this method returns whether
   * a native control is actually being used.
   */
  bool nativeControl() const;

  /*! \brief Sets the slider orientation.
   *
   * \sa orientation()
   */
  void setOrientation(Wt::Orientation orientation);

  /*! \brief Returns the slider orientation.
   *
   * \sa setOrientation()
   */
  Orientation orientation() const { return orientation_; }

  /*! \brief Sets the tick interval.
   *
   * The tick interval specifies the interval for placing ticks along
   * the slider. The interval is specified in value units (not pixel
   * units). A value of 0 specifies an automatic tick interval, which
   * defaults to 3 ticks spanning the whole range.
   *
   * \sa tickInterval(), setTickPosition()
   */
  void setTickInterval(int tickInterval);

  /*! \brief Returns the tick interval.
   *
   * \sa setTickInterval()
   */
  int tickInterval() const { return tickInterval_; }

  /*! \brief Sets the tick position.
   *
   * The tick position indicates if and where ticks are placed around the
   * slider groove.
   *
   * \sa tickPosition(), setTickInterval()
   */
  void setTickPosition(WFlags<TickPosition> tickPosition);

  /*! \brief Returns the tick position.
   *
   * \sa setTickPosition(), setTickInterval()
   */
  WFlags<TickPosition> tickPosition() const { return tickPosition_; }

  /*! \brief Sets the slider value.
   *
   * The value is automatically trimmed to the valid range (minimum()
   * to maximum()).
   *
   * \sa value()
   */
  virtual void setValue(int value);

  /*! \brief Returns the current slider value.
   *
   * \sa setValue()
   */
  int value() const { return value_; }

  /*! \brief Sets the maximum value.
   *
   * The maximum value defines the upper limit of the valid range. The
   * lower limit and current value are automatically adjusted to
   * remain valid.
   *
   * \sa maximum(), setMinimum(), setRange()
   */
  void setMaximum(int maximum);

  /*! \brief Returns the maximum value.
   *
   * \sa setMaximum(int)
   */
  int maximum() const { return maximum_; }

  /*! \brief Sets the minimum value.
   *
   * The minimum value defines the lower limit of the valid range. The
   * upper limit and current value are automatically adjusted to
   * remain valid.
   *
   * \sa minimum(), setMaximum(), setRange()
   */
  void setMinimum(int minimum);

  /*! \brief Returns the minimum value.
   *
   * \sa setMinimum(int)
   */
  int minimum() const { return minimum_; }

  /*! \brief Sets the value range.
   *
   * \sa setMinimum(), setMaximum()
   */
  void setRange(int minimum, int maximum);

  /*! \brief %Signal emitted when the user has changed the value of the
   *         slider.
   *
   * The new value is passed as the argument.
   *
   * \sa sliderMoved()
   */
  Signal<int>& valueChanged() { return valueChanged_; }

  /*! \brief %Signal emitted while the user drags the slider.
   *
   * The current dragged position is passed as the argument. Note that the
   * slider value is not changed while dragging the slider, but only after
   * the slider has been released.
   *
   * \sa valueChanged()
   */
  JSignal<int>& sliderMoved() { return sliderMoved_; }

  /*! \brief Sets the slider handle width.
   *
   * This sets the width for the handle, which is needed to accurately
   * position the handle.
   *
   * The default value is 20 pixels.
   */
  void setHandleWidth(int width);

  /*! \brief Returns the handle width.
   *
   * \sa setHandleWidth()
   */
  int handleWidth() const { return handleWidth_; }

  virtual void setDisabled(bool disabled) override;
  virtual void resize(const WLength& width, const WLength& height) override;
  virtual WT_USTRING valueText() const override;
  virtual void setValueText(const WT_USTRING& value) override;
  virtual void enableAjax() override;

protected:
  /*! \brief Paints a slider ticks (for a non-native widget)
   *
   * The default implementation draws ticks taking into account the
   * the tickPosition.
   *
   * The mid point for the tick should be at position (x, y). The \p
   * value that corresponds to the tick is also passed.
   */
  virtual void paintTick(WPainter& painter, int value, int x, int y);

  /*! \brief Creates the handle (for a non-native widget)
   *
   * The default implementation creates a container widget. You may
   * want to specialize this function if you want to have more control
   * on the handle appearance or if you want to associate with the
   * handle a tooltip or other information (e.g. a popup balloon).
   */
  virtual std::unique_ptr<WInteractWidget> createHandle();

  virtual void signalConnectionsChanged() override;
  virtual void layoutSizeChanged(int width, int height) override;

  virtual void render(WFlags<RenderFlag> flags) override;
  virtual void updateDom(DomElement& element, bool all) override;
  virtual DomElementType domElementType() const override;
  virtual void setFormData(const FormData& formData) override;

private:
  Orientation          orientation_;
  int                  tickInterval_;
  WFlags<TickPosition> tickPosition_;
  bool                 preferNative_, changed_, changedConnected_;
  int                  handleWidth_;

  int                  minimum_, maximum_;
  int                  value_;

  Signal<int>          valueChanged_;
  JSignal<int>         sliderMoved_;

  std::unique_ptr<PaintedSlider> paintedSlider_;

  void update();
  void onChange();

  friend class PaintedSlider;
};

W_DECLARE_OPERATORS_FOR_FLAGS(WSlider::TickPosition)

}

#endif // WSLIDER_H_
