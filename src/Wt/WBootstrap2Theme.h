// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2020 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_WBOOTSTRAP2_THEME_H
#define WT_WBOOTSTRAP2_THEME_H

#include <Wt/WTheme.h>

namespace Wt {

/*! \class WBootstrap2Theme Wt/WBootstrap2Theme.h Wt/WBootstrap2Theme.h
 *  \brief Theme based on the Twitter Bootstrap 2 CSS framework.
 *
 * This theme implements support for building a %Wt web application
 * that uses Twitter Bootstrap as a theme for its (layout and)
 * styling. The theme comes with CSS from Bootstrap version 2.2.2.
 * Only the CSS components of twitter bootstrap are used,
 * but not the JavaScript (i.e. the functional parts), since the
 * functionality is already built-in to the widgets from the library.
 *
 * Using this theme, various widgets provided by the library are
 * rendered using markup that is compatible with Twitter
 * Bootstrap. The bootstrap theme is also extended with a proper
 * (compatible) styling of widgets for which bootstrap does not
 * provide styling (table views, tree views, sliders, etc...).
 *
 * By default, the theme will use CSS resources that are shipped
 * together with the %Wt distribution, but since the Twitter Bootstrap
 * CSS API is a popular API for custom themes, you can easily replace the
 * CSS with custom-built CSS (by reimplementing styleSheets()).
 *
 * Although this theme facilitates the use of Twitter Bootstrap with
 * %Wt, it is still important to understand how Bootstrap expects
 * markup to be, especially related to layout using its grid system, for
 * which we refer to the official bootstrap documentation, see
 * http://getbootstrap.com
 *
 * \sa WApplication::setTheme()
 */
class WT_API WBootstrap2Theme : public WTheme {
public:
  /*! \brief Constructor.
   */
  WBootstrap2Theme();

  virtual ~WBootstrap2Theme() override;

  /*! Enables responsive features.
   *
   * Responsive features can be enabled only at application startup.
   *
   * Responsive features are disabled by default.
   */
  void setResponsive(bool responsive);

  /*! \brief Returns whether responsive features are enabled.
   *
   * \sa setResponsive()
   */
  bool responsive() const { return responsive_; }

  virtual std::string name() const override;
  virtual std::string resourcesUrl() const override;
  virtual std::vector<WLinkedCssStyleSheet> styleSheets() const override;
  virtual void init(WApplication *app) const override;
  virtual void apply(WWidget *widget, WWidget *child, int widgetRole) const override;
  virtual void apply(WWidget *widget, DomElement &element, int elementRole) const override;
  virtual std::string disabledClass() const override;
  virtual std::string activeClass() const override;
  virtual std::string utilityCssClass(int utilityCssClassRole) const override;
  virtual bool canStyleAnchorAsButton() const override;
  virtual void applyValidationStyle(WWidget *widget,
                                    const WValidator::Result &validation,
                                    WFlags<ValidationStyleFlag> styles) const override;
  virtual bool canBorderBoxElement(const DomElement &element) const override;

private:
  bool responsive_;
};

}

#endif // WT_WBOOTSTRAP2_THEME_H
