// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WOVERLAY_LOADING_INDICATOR_H_
#define WOVERLAY_LOADING_INDICATOR_H_

#include <Wt/WContainerWidget.h>
#include <Wt/WLoadingIndicator.h>

namespace Wt {

class WText;

/*! \class WOverlayLoadingIndicator Wt/WOverlayLoadingIndicator.h Wt/WOverlayLoadingIndicator.h
 *  \brief A more obvious loading indicator that grays the window.
 *
 * This loading indicator uses a gray semi-transparent overlay to
 * darken the window contents, and centers a loading icon (with some
 * text).
 *
 * Usage example:
 * \if cpp
 * \code
 * Wt::WApplication *app = Wt::WApplication::instance();
 * app->setLoadingIndicator(std::make_unique<Wt::WOverlayLoadingIndicator>());
 * \endcode
 * \endif
 *
 * \image html WOverlayLoadingIndicator.png "The overlay loading indicator" 
 *
 * \note For this loading indicator to render properly in IE, you need to
 *       reset the "body" margin to 0. Using the inline stylesheet, this could
 *       be done using:
 * \if cpp
 * \code
 *   WApplication::instance()->styleSheet().addRule("body", "margin: 0px");
 * \endcode
 * \endif
 *
 * <h3>CSS</h3>
 *
 * This widget does not provide styling, and can be styled using
 * inline or external CSS as appropriate.
 *
 * <h3>i18n</h3>
 *
 * The strings used in this class can be translated by overriding
 * the default values for the following localization keys:
 * - Wt.WOverlayLoadingIndicator.Loading: Loading...
 *
 * \sa WApplication::setLoadingIndicator()
 */
class WT_API WOverlayLoadingIndicator : public WLoadingIndicator
{
public:
  /*! \brief Construct the loading indicator.
   *  \param styleClass the style class for the central box
   *  \param backgroundStyleClass the style class for the "background" part
   *         of the indicator
   *  \param textStyleClass the style class for the text that is displayed
   *
   *  \note if styleClass is not set, the central box gets the CSS style
   *        elements
   *        \code
   *             background: white;
   *             border: 3px solid #333333;
   *             z-index: 10001; visibility: visible;
   *             position: absolute; left: 50%; top: 50%;
   *             margin-left: -50px; margin-top: -40px;
   *             width: 100px; height: 80px;
   *             font-family: arial,sans-serif;
   *             text-align: center
   *        \endcode
   *  \note if backgroundStyleClass is not set, the background gets the CSS
   *        style elements
   *        \code
   *             background: #DDDDDD;
   *             height: 100%; width: 100%;
   *             top: 0px; left: 0px;
   *             z-index: 10000;
   *             -moz-background-clip: -moz-initial;
   *             -moz-background-origin: -moz-initial;
   *             -moz-background-inline-policy: -moz-initial;
   *             opacity: 0.5; filter: alpha(opacity=50); -moz-opacity:0.5;
   *             position: absolute;
   *        \endcode
   */
  WOverlayLoadingIndicator(const WT_USTRING& styleClass = WT_USTRING(),
			   const WT_USTRING& backgroundStyleClass = WT_USTRING(),
			   const WT_USTRING& textStyleClass = WT_USTRING());

  virtual void setMessage(const WString& text) override;

private:
  WContainerWidget *cover_;
  WContainerWidget *center_;
  WText            *text_;
};

}

#endif // WOVERLAY_LOADING_INDICATOR_H_
