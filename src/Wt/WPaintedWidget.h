// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WPAINTED_WIDGET_H_
#define WPAINTED_WIDGET_H_

#include <Wt/WInteractWidget.h>
#include <Wt/WJavaScript.h>
#include <Wt/WJavaScriptExposableObject.h>
#include <Wt/WJavaScriptHandle.h>
#include <Wt/WJavaScriptSlot.h>

#include <Wt/WJavaScriptObjectStorage.h>

namespace Wt {

class WAbstractArea;
class WImage;
class WPaintDevice;
class WWidgetPainter;

/*! \brief Enumeration that indicates a client-side rendering method for a
 *         painted widget.
 *
 * \sa WPaintedWidget
 */
enum class RenderMethod {
  /*! \brief SVG (Most browsers) or VML (Internet Explorer < 9) embedded in
   *         the page.
   */
  InlineSvgVml,

  /*! \brief The HTML5 canvas element.
   */
  HtmlCanvas,

  /*! \brief Using a PNG image resource.
   */
  PngImage
};

/*! \class WPaintedWidget Wt/WPaintedWidget.h Wt/WPaintedWidget.h
 *  \brief A widget that is painted using vector graphics.
 *
 * A painted widget is rendered from basic drawing
 * primitives. Rendering is done not on the server but on the browser,
 * using different rendering methods:
 *
 * <table>
 *   <tr><td><b>Browser</b></td><td><b>Methods</b></td>
 *       <td><b>Default method</b></td></tr>
 *   <tr><td>Firefox 1.5+</td><td>HtmlCanvas, InlineSVG, PngImage</td>
 *       <td>HtmlCanvas</td></tr>
 *   <tr><td>Internet Explorer 6.0+</td><td>InlineVML, PngImage</td>
 *       <td>InlineVML</td></tr>
 *   <tr><td>Internet Explorer 9+</td><td>HtmlCanvas, InlineSVG, PngImage</td>
 *       <td>HtmlCanvas</td></tr>
 *   <tr><td>Safari</td><td>HtmlCanvas, InlineSVG, PngImage</td>
 *       <td>HtmlCanvas</td></tr>
 *   <tr><td>Opera</td><td>InlineSVG, HtmlCanvas*, PngImage</td>
 *       <td>InlineSVG</td></tr>
 *   <tr><td>other</td><td>?</td><td>HtmlCanvas, PngImage</td></tr>
 * </table>
 *
 * <i>* HtmlCanvas occasionally suffers from rendering artefacts in Opera.</i>
 *
 * The different rendering methods correspond to different
 * WPaintDevice implementations, from which this widget choses a
 * suitable one depending on the browser capabilities and configuration.
 *
 * If no JavaScript is available, the JavaScript-based HtmlCanvas will
 * not be used, and InlineSVG will be used instead. The method used
 * may be changed by using setPreferredMethod().
 *
 * In some browsers, InlineSVG requires that the document is rendered as
 * XHTML. This must be enabled in the configuration file using the
 * <tt>&lt;send-xhtml-mime-type&gt;</tt> option. By default, this
 * option is off. Firefox 4 and Chrome do support svg in normal html mode.
 *
 * The PngImage is the most portable rendering method, and may be the
 * fastest if the painting is of high complexity and/or the image is
 * fairly small.
 *
 * To use a %WPaintedWidget, you must derive from it and reimplement
 * paintEvent(WPaintDevice *paintDevice). To paint on a WPaintDevice,
 * you will need to use a WPainter. Repainting is triggered by calling
 * the update() method.
 *
 * \if cpp
 * Usage example:
 * \code
 * class MyPaintedWidget : public Wt::WPaintedWidget
 * {
 * public:
 *   MyPaintedWidget()
 *     : Wt::WPaintedWidget()
 *       foo_(100)
 *   {
 *      resize(200, 200); // provide a default size
 *   }
 *
 *   void setFoo(int foo) {
 *      foo_ = foo;
 *      update(); // trigger a repaint
 *   }
 *
 * protected:
 *   void paintEvent(Wt::WPaintDevice *paintDevice) {
 *     Wt::WPainter painter(paintDevice);
 *     painter.drawLine(20, 20, foo_, foo_);
 *     ...
 *   }
 *
 * private:
 *   int foo_;
 * };
 * \endcode
 * \endif
 * 
 * <h3>CSS</h3>
 *
 * Styling through CSS is not applicable.
 *
 * \note A %WPaintedWidget requires that it is given a size using resize() or
 *       by a layout manager.
 *
 * <h3>Client side interaction and repainting</h3>
 *
 * If the widget is drawn as an HTML canvas element, i.e. the \link getMethod() method\endlink
 * is HtmlCanvas, a %WPaintedWidget can expose certain objects to be modified client side.
 *
 * \sa WJavaScriptHandle, WJavaScriptExposableObject
 *
 * \sa WImage
 *
 * \ingroup painting
 */
class WT_API WPaintedWidget : public WInteractWidget
{
public:
  /*! \brief Create a new painted widget.
   */
  WPaintedWidget();

  /*! \brief Destructor.
   */
  ~WPaintedWidget();

  /*! \brief Sets the preferred rendering method.
   *
   * When \p method is supported by the browser, then it is chosen
   * for rendering.
   */
  void setPreferredMethod(RenderMethod method);

  /*! \brief Returns the preferred rendering method.
   *
   * \sa setPreferredMethod(Method)
   */
  RenderMethod preferredMethod() const { return preferredMethod_; }

  /*! \brief Lets the widget repaint itself.
   *
   * Repainting is not immediate, but happens after when the event loop
   * is exited.
   *
   * Unless a Wt::PaintFlag::Update paint flag is set, the widget is first
   * cleared.
   */
  void update(WFlags<PaintFlag> flags = None);

  virtual void resize(const WLength& width, const WLength& height) override;

  /*! \brief Adds an interactive area.
   *
   * Adds the \p area which listens to events in a specific region
   * of the widget. Areas are organized in a list, to which the given
   * \p area is appended. When areas overlap, the area with the
   * lowest index receives the event.
   *
   * \note When defining at least one area, no more events will
   * propagate to the widget itself. As a work-around, you can emulate
   * this by listening for events on a WRectArea that corresponds to
   * the whole widget, and which is added as the last area (catching
   * all events that were not caught by preceding areas).
   *
   * \sa insertArea(int, WAbstractArea *)
   */
  void addArea(std::unique_ptr<WAbstractArea> area);

  /*! \brief Inserts an interactive area.
   *
   * Inserts the \p area which listens to events in the
   * coresponding area of the widget. Areas are organized in a list,
   * and the <i>area</i> is inserted at index \p index. When areas
   * overlap, the area with the lowest index receives the event.
   *
   * \note When defining at least one area, no more events will
   * propagate to the widget itself. As a work-around, you can emulate
   * this by listening for events on a WRectArea that corresponds to
   * the whole widget, and which is added as the last area (catching
   * all events that were not caught by preceding areas).
   *
   * \sa addArea(WAbstractArea *)
   */
  void insertArea(int index, std::unique_ptr<WAbstractArea> area);

  /*! \brief Removes an interactive area.
   *
   * Removes the \p area from this widget.
   *
   * \sa addArea(WAbstractArea *)
   */
  std::unique_ptr<WAbstractArea> removeArea(WAbstractArea *area);

  /*! \brief Returns the interactive area at the given index.
   *
   * Returns \c 0 if \p index was invalid.
   *
   * \sa insertArea(int, WAbstractArea *)
   */
  WAbstractArea *area(int index) const;

  /*! \brief Returns the interactive areas set for this widget.
   *
   * \sa addArea()
   */
  const std::vector<WAbstractArea *> areas() const;

  /*! \brief A JavaScript slot that repaints the widget when triggered.
   *
   * This is useful for client-side initiated repaints. You may want to use this
   * if you want to add interaction or animation to your WPaintedWidget.
   *
   * \note This feature is currently only supported if the
   *	   \link getMethod() method\endlink is HtmlCanvas.
   *	   This will not cause a server roundtrip. Instead, the
   *	   resulting JavaScript of paintEvent() will be re-executed
   *	   on the client side.
   *
   * \sa objJsRef()
   */
  JSlot &repaintSlot() { return repaintSlot_; }

protected:
  /*! \brief Create a WTransform that is accessible from JavaScript,
   *         associated with this %WPaintedWidget
   */
  WJavaScriptHandle<WTransform> createJSTransform();

  /*! \brief Create a WBrush that is accessible from JavaScript,
   *         associated with this %WPaintedWidget
   */
  WJavaScriptHandle<WBrush> createJSBrush();

  /*! \brief Create a WPen that is accessible from JavaScript,
   *         associated with this %WPaintedWidget
   */
  WJavaScriptHandle<WPen> createJSPen();

  /*! \brief Create a WPainterPath that is accessible from JavaScript,
   *	     associated with this %WPaintedWidget
   */
  WJavaScriptHandle<WPainterPath> createJSPainterPath();

  /*! \brief Create a WRectF that is accessible from JavaScript,
   *	     associated with this %WPaintedWidget
   */
  WJavaScriptHandle<WRectF> createJSRect();

  /*! \brief Create a WPointF that is accessible from JavaScript,
   *	     associated with this %WPaintedWidget
   */
  WJavaScriptHandle<WPointF> createJSPoint();

  virtual void layoutSizeChanged(int width, int height) override;

  /*! \brief Returns the actual method used for rendering.
   *
   * The default method considers browser capabilites and the preferred
   * method to make an actual choice for the implementation.
   *
   * You may want to reimplement this method to override this choice.
   */
  virtual RenderMethod getMethod() const;

  /*! \brief Paints the widget.
   *
   * You should reimplement this method to paint the contents of the widget,
   * using the given paintDevice.
   */
  virtual void paintEvent(WPaintDevice *paintDevice) = 0;

  /*! \brief Creates a paint device.
   *
   * Although it's usually not necessary to call this function, you may
   * want to reimplement this function to customize or specialize the
   * device used for painting the widget.
   */
  virtual std::unique_ptr<WPaintDevice> createPaintDevice() const;

  virtual DomElementType domElementType() const override;
  virtual void updateDom(DomElement& element, bool all) override;
  virtual DomElement *createDomElement(WApplication *app) override;
  virtual void getDomChanges(std::vector<DomElement *>& result,
			     WApplication *app) override;
  virtual void propagateRenderOk(bool deep) override;
  virtual void enableAjax() override;
  virtual void render(WFlags<RenderFlag> flags) override;
  virtual void setFormData(const FormData& formData) override;

protected:
  /*! \brief Returns a JavaScript reference to the client side representation of the WPaintedWidget
   *
   * The client side representation exposes the following interface:
   * \code
   * {
   *   canvas: exposes the underlying HTML canvas element
   *   repaint: a function that, when called, will repaint the widget without a server roundtrip
   * }
   * \endcode
   *
   * \note The \link getMethod() method\endlink should be HtmlCanvas and
   *	   there has to be at least one WJavaScriptHandle associated with
   *	   this WPaintedWidget in order for this reference to be valid.
   */
  std::string objJsRef() const;

private:
  void defineJavaScript();

  RenderMethod preferredMethod_;
  std::unique_ptr<WWidgetPainter> painter_;
  bool needRepaint_, sizeChanged_, areaImageAdded_;
  WFlags<PaintFlag> repaintFlags_;
  std::unique_ptr<WImage> areaImage_;
  int renderWidth_, renderHeight_;

  JSlot repaintSlot_;
  WJavaScriptObjectStorage jsObjects_;
  bool              jsDefined_;

  void resizeCanvas(int width, int height);
  bool createPainter();
  void createAreaImage();

  friend class WWidgetVectorPainter;
  friend class WWidgetCanvasPainter;
  friend class WWidgetRasterPainter;
};

}

#endif // WPAINTED_WIDGET_H_
