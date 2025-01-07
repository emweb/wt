// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WICONPAIR_H_
#define WICONPAIR_H_

#include <Wt/WCompositeWidget.h>
#include <Wt/WIcon.h>
#include <Wt/WImage.h>
#include <Wt/WEvent.h>

namespace Wt {


/*! \class WIconPair Wt/WIconPair.h Wt/WIconPair.h
 *  \brief A widget that shows one of two icons depending on its state.
 *
 * This is a utility class that simply manages two images, only one of
 * which is shown at a single time, which reflects the current
 * 'state'.
 *
 * The widget may react to click events, by changing state.
 *
 * <h3>CSS</h3>
 *
 * This widget does not provide styling,
 * and can be styled using inline or external CSS as appropriate.
 * The image may be styled via the <tt>&lt;img&gt;</tt> elements.
 */
class WT_API WIconPair : public WCompositeWidget
{
public:

  /*! \brief An enumeration describing a type of icon
   *
   * Each icon type describe how the string representing the icon
   * should be interpreted. Depending on the interpretation the icon
   * is looked for in different places, or within different resources.
   * 
   * Under \p URI, a static image is expected. This can be linked in many
   * ways, using absolute paths, relative paths, or even external resources.
   * The images can thus be served by Wt itself (in its docroot), or be
   * retrieved from another server.
   * 
   * With \p IconName, a simple string is expected, matching the format of
   * the Font Awesome version you are using. In this case it is required to
   * include the stylesheet (WApplication::useStyleSheet()) in your application
   * before this can be used.
   */
  enum class IconType {
    URI,      //!< The URI of an image, linking to a static resource.
    IconName  //!< The name of a Font Awesome icon, using an external bundle.
  };

  /*! \brief Construct an icon pair from the two icons.
   *
   * The constructor takes the URL or the Font Awesome name of the 
   * two icons. When \p clickIsSwitch is set \c true, clicking on 
   * the icon will switch state.
   * 
   * \note The Font Aweswome name can be followed by sizing
   *       information separated by a space if the Font Aweswome
   *       version used allows it. A valid icon can look like this for
   *       example: \p "fa-solid fa-camera fa-1x", or
   *       \p "fa-solid fa-camera fa-sm" (valid for version v6.7.2).
   *       More sizing information can be found on
   *       https://docs.fontawesome.com/web/style/size.
   * 
   * \warning By default, the strings are considered to represent
   *          the URIs of the icons. Use setIcon1Type() and 
   *          setIcon2Type() to change the IconType accordingly if 
   *          it is not what the string represents. Both can be set
   *          at once using setIconTypes().
   * 
   * \sa setIcon1Type(IconType), setIcon2Type(IconType)
   */
  WIconPair(const std::string& icon1Str, const std::string& icon2Str,
            bool clickIsSwitch = true);

  /*! \brief Sets the IconType of the first icon
   * 
   * \sa setIconsType(IconType)
   */
  void setIcon1Type(IconType type);

  /*! \brief Sets the IconType of the second icon
   * 
   * \sa setIconsType(IconType)
   */
  void setIcon2Type(IconType type);

  /*! \brief Sets the IconType of the both icons
   * 
   * Sets the IconType of both icons. The icon type should be
   * IconType::URI if the URI of the icon was given at 
   * construction. If the name of a Font Awesome icon was given, 
   * the icon type should be set to IconType::IconName instead.
   * 
   * By default this will be set to IconType::URI.
   * 
   * \sa setIcon1Type(IconType), setIcon2Type(IconType)
   */
  void setIconsType(IconType type);

  /*! \brief Sets the state, which determines the visible icon.
   *
   * The first icon has number 0, and the second icon has number 1.
   *
   * The default state is 0.
   *
   * \sa state()
   */
  void setState(int num);

  /*! \brief Returns the current state.
   *
   * \sa setState()
   */
  int state() const;

  /*! \brief Returns the first icon image
   * 
   * \deprecated Icon can now be different widget type. Use uriIcon1()
   *  or iconNameIcon1() instead.
   */
  WT_DEPRECATED("Icon can now be different widget type. Use uriIcon1() or iconNameIcon1() instead.")
  WImage *icon1() const { return uriIcon1(); }

  /*! \brief Returns the second icon image
   * 
   * \deprecated Icon can now be different widget type. Use uriIcon2()
   *  or iconNameIcon2() instead.
   */
  WT_DEPRECATED("Icon can now be different widget type. Use uriIcon2() or iconNameIcon2() instead.")
  WImage *icon2() const { return uriIcon2(); }

  /*! \brief Returns the first icon as WImage
   * 
   * If first icon type is IconType::URI returns the first icon as WImage,
   * otherwise returns nullptr.
   * 
   * \sa iconNameIcon1()
   */
  WImage *uriIcon1() const { return image_[0]; }

  /*! \brief Returns the second icon as WImage
   * 
   * If second icon type is IconType::URI returns the second icon as WImage,
   * otherwise returns nullptr.
   * 
   * \sa iconNameIcon2()
   */
  WImage *uriIcon2() const { return image_[1]; }

  /*! \brief Returns the first icon as WIcon
   * 
   * If first icon type is IconType::IconName returns the first icon as WIcon,
   * otherwise returns nullptr.
   * 
   * \sa uriIcon1()
   */
  WIcon *iconNameIcon1() const { return wicon_[0]; }

  /*! \brief Returns the second icon as WIcon
   * 
   * If second icon type is IconType::IconName returns the second icon as WIcon,
   * otherwise returns nullptr.
   * 
   * \sa uriIcon2()
   */
  WIcon *iconNameIcon2() const { return wicon_[1]; }
  
  /*! \brief Sets the state to 0 (show icon 1).
   *
   * \sa setState(int)
   */
  void showIcon1();

  /*! \brief Sets the state to 1 (show icon 2).
   *
   * \sa setState(int)
   */
  void showIcon2();

  /*! \brief %Signal emitted when clicked while in state 0 (icon 1 is
   *         shown).
   *
   * Equivalent to:
   * \code
   * icon1()->clicked()
   * \endcode
   */
  EventSignal<WMouseEvent>& icon1Clicked();

  /*! \brief %Signal emitted when clicked while in state 1 (icon 2 is
   *         shown).
   *
   * Equivalent to:
   * \code
   * icon2()->clicked()
   * \endcode
   */
  EventSignal<WMouseEvent>& icon2Clicked();

private:
  const bool clickIsSwitch_;
  WContainerWidget *impl_;
  std::string iconStr_[2];
  WIcon *wicon_[2];
  WImage *image_[2];

  WInteractWidget *usedIcon(int i) const;
  WInteractWidget *usedIcon1() const { return usedIcon(0); }
  WInteractWidget *usedIcon2() const { return usedIcon(1); }
  void resetIcon(int i, IconType type);
};

}

#endif // WICONPAIR_H_
