// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2020 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_WBOOTSTRAP5_THEME_H
#define WT_WBOOTSTRAP5_THEME_H

#include <Wt/WTheme.h>

namespace Wt {

/*! \class WBootstrap5Theme Wt/WBootstrap5Theme.h Wt/WBootstrap5Theme.h
 *  \brief Theme based on the Bootstrap 5 CSS framework.
 *
 * This theme implements support for building a %Wt web application
 * that uses Bootstrap 5 as a theme for its (layout and) styling.
 *
 * Using this theme, various widgets provided by the library are
 * rendered using markup that is compatible with Bootstrap 5.
 * The bootstrap theme is also extended with a proper
 * (compatible) styling of widgets for which bootstrap does not
 * provide styling (table views, tree views, sliders, etc...).
 *
 * By default, the theme will use CSS and JavaScript resources that are shipped
 * together with the %Wt distribution, but you can replace the CSS with
 * custom-built CSS by reimplementing styleSheets().
 *
 * Although this theme styles individual widgets correctly, for your web application's
 * layout you are recommended to use WTemplate in conjunction with Bootstrap's CSS classes.
 * For this we refer to Bootstrap's documentation at https://getbootstrap.com.
 *
 * \sa WApplication::setTheme()
 */
class WT_API WBootstrap5Theme : public WTheme {
public:
  /*! \brief Constructor.
   */
  WBootstrap5Theme();

  ~WBootstrap5Theme() override;

  void init(WApplication *app) const override;

  std::string name() const override;
  std::string resourcesUrl() const override;
  std::vector<WLinkedCssStyleSheet> styleSheets() const override;
  void apply(WWidget *widget, WWidget *child, int widgetRole) const override;
  void apply(WWidget *widget, DomElement &element, int elementRole) const override;
  void setDataTarget(WWidget *widget, WWidget *target) const override;
  std::string disabledClass() const override;
  std::string activeClass() const override;
  std::string utilityCssClass(int utilityCssClassRole) const override;
  bool canStyleAnchorAsButton() const override;
  void applyValidationStyle(WWidget *widget,
                            const Wt::WValidator::Result &validation,
                            WFlags<ValidationStyleFlag> flags) const override;
  bool canBorderBoxElement(const DomElement &element) const override;

private:
  static std::string classBtn(const WWidget *widget);
  static bool hasButtonStyleClass(const WWidget *widget);
  static bool hasNavbarExpandClass(const WNavigationBar *widget);
};

}

#endif // WT_WBOOTSTRAP5_THEME_H
