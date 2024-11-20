// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2013 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WICON_H_
#define WICON_H_

#include <Wt/WInteractWidget.h>

namespace Wt {

/*! \class WIcon Wt/WIcon.h Wt/WIcon.h
 *  \brief A widget that represents a Font-Aweswome icon.
 *
 * By default, Wt will load the default Font-Awesome included with it.
 * This is version 4.3.0. For a list of all icons, visit:
 * https://fontawesome.com/v4/icons/
 *
 * \see setName()
 */
class WT_API WIcon : public WInteractWidget
{
public:
  //! Creates an empty icon.
  WIcon();

  /*! \brief Creates an icon with the given name.
   *
   * \see setName()
   */
  WIcon(const std::string& name);

  /*! \brief Set the icon name.
   *
   * This sets the name of the icon. The name should be a the name of a
   * Font-Aweswome icon, without the `fa-` prefix.
   *
   * Usage example:
   * The "play" icon: https://fontawesome.com/v4/icon/play
   * can be included with:
   * \if cpp
   * \code
   *  auto app = Wt::WApplication::instance();
   *  app->root()->addNew<WIcon>("play");
   * \endcode
   * \elseif java
   * \code
   *  WApplication app = WApplication.getInstance();
   *  app.getRoot().addWidget(new WIcon("play"));
   * \endcode
   * \endif
   *
   * \note The name can be followed by sizing information
   *       separated by a space if the Font Aweswome version
   *       used allows it. E.g. \p "play fa-4"
   */
  void setName(const std::string& name);

  /*! \brief Returns the icon name.
   *
   * \see setName()
   */
  std::string name() const { return name_; }

  /*! \brief Changes the icon's size.
   *
   * \note This is done in CSS, not using the `fa-{size}` method.
   */
  void setSize(double factor);

  /*! \brief Returns the icon size.
   *
   * \see setSize()
   */
  double size() const;

  /*! \brief Loads the Font-Aweswome css style sheet.
   *
   * This is a convenience function that adds Font-Aweswome's CSS style
   * sheet to the list of used style sheets.
   *
   * By default this will load the stylesheet present at:
   * \p resources/font-awesome/css/font-awesome.min.css
   * The \p resources directory can be set with a command-line option,
   * namely \p --resources-dir,
   * see <a href="https://www.webtoolkit.eu/wt/doc/reference/html/overview.html#config_wthttpd">Wt's configuration options</a>.
   *
   * \note This is automatically called when needed by
   *       WIcon.
   */
  static void loadIconFont();

protected:
  virtual void updateDom(DomElement& element, bool all) override;
  virtual DomElementType domElementType() const override;
  virtual void propagateRenderOk(bool deep) override;

private:
  std::string name_;
  bool iconChanged_;
};

}

#endif // WICON_H_
