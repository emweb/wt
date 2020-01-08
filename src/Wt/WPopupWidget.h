// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2012 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WPOPUP_WIDGET_H_
#define WPOPUP_WIDGET_H_

#include <Wt/WCompositeWidget.h>

namespace Wt {

/*! \class WPopupWidget Wt/WPopupWidget.h Wt/WPopupWidget.h
 *  \brief Base class for popup widgets.
 *
 * A popup widget anchors to another widget, for which it usually
 * provides additional information or assists in editing, etc...
 *
 * The popup widget will position itself relative to the anchor widget
 * by taking into account available space, and switching sides if
 * necessary to fit the widget into the current window. For example, a
 * vertically anchored widget will by default be a "drop-down",
 * positioning itself under the anchor widget, but it may also choose
 * to position itself above the anchor widget if space is lacking
 * below.
 *
 * \if cpp
 * Before Wt 4, WPopupWidget had a setDeleteWhenHidden(bool) function,
 * causing the WPopupWidget to be deleted when it's hidden. This function
 * has been removed. To achieve the same effect, the following code can be used:
 *
 * \code
 * // Assuming that popupWidget_ is a std::unique_ptr<WPopupWidget>, and a
 * // member of "this".
 * popupWidget_->hidden().connect([this](){ popupWidget_.reset(); });
 * \endcode
 * \endif
 */
class WT_API WPopupWidget : public WCompositeWidget
{
public:
  /*! \brief Constructor.
   *
   * You need to pass in a widget that provides the main contents of the
   * widget (e.g. a WTemplate or WContainerWidget).
   *
   * Unlike other widgets, a popup widget is a top-level widget that should
   * not be added to another container.
   */
  WPopupWidget(std::unique_ptr<WWidget> impl);

  /*! \brief Destructor.
   */
  virtual ~WPopupWidget();

  /*! \brief Sets an anchor widget.
   *
   * A vertical popup will show below (or above) the widget, while a
   * horizontal popup will show right (or left) of the widget.
   */
  void setAnchorWidget(WWidget *widget,
                       Orientation orientation = Orientation::Vertical);

  /* \brief Returns the anchor widget.
   *
   * \sa setAnchorWidget()
   */
  WWidget *anchorWidget() const { return anchorWidget_.get(); }

  /*! \brief Returns the orientation.
   *
   * \sa setOrientation()
   */
  Orientation orientation() const { return orientation_; }

  /*! \brief Sets transient property.
   *
   * A transient popup will automatically hide when the user clicks
   * outside of the popup. When \p autoHideDelay is not 0, then it
   * will also automatically hide when the user moves the mouse
   * outside the widget for longer than this delay (in ms).
   */
  void setTransient(bool transient, int autoHideDelay = 0);

  /*! \brief Returns whether the popup is transient.
   *
   * \sa setTransient()
   */
  bool isTransient() const { return transient_; }

  /*! \brief Returns the auto-hide delay.
   *
   * \sa setTransient()
   */
  int autoHideDelay() const { return autoHideDelay_; }

  virtual void setHidden(bool hidden,
                         const WAnimation& animation = WAnimation()) override;

  /*! \brief %Signal emitted when the popup is hidden.
   *
   * This signal is emitted when the popup is being hidden because of a
   * client-side event (not when setHidden() or hide() is called).
   */
  Signal<>& hidden() { return hidden_; }

  /*! \brief %Signal emitted when the popup is shown.
   *
   * This signal is emitted when the popup is being shown because of a
   * client-side event (not when setHidden() or show() is called).
   */
  Signal<>& shown() { return shown_; }

protected:
  virtual void render(WFlags<RenderFlag> flags) override;
  virtual void onPathChange();

private:
  observing_ptr<WWidget> anchorWidget_;
  Orientation orientation_;
  bool transient_;
  int autoHideDelay_;
  Signal<> hidden_, shown_;
  JSignal<> jsHidden_, jsShown_;

  void create(WWidget *parent);
  void defineJS();
};

}

#endif // WPOPUP_WIDGET_H_
