// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2012 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_WBOOTSTRAP_THEME_H
#define WT_WBOOTSTRAP_THEME_H

#include <Wt/WTheme.h>

#include <string>

namespace Wt {

/*!
 * \brief Enumeration to select a bootstrap version.
 *
 * \sa setVersion()
 */
enum class BootstrapVersion {
  v2 = 2, //!< Bootstrap 2
  v3 = 3  //!< Bootstrap 3
};

/*! \class WBootstrapTheme Wt/WBootstrapTheme.h Wt/WBootstrapTheme.h
 *  \brief Theme based on the Twitter Bootstrap CSS framework.
 *
 * This theme implements support for building a %Wt web application
 * that uses Twitter Bootstrap as a theme for its (layout and)
 * styling. The theme comes with CSS from Bootstrap version 2.2.2 or
 * version 3.1. Only the CSS components of twitter bootstrap are used,
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
class WT_API WBootstrapTheme : public WTheme
{
public:
  /*! \brief Typedef for enum Wt::BootstrapVersion */
  typedef BootstrapVersion Version;

  /*! \brief Constructor.
   */
  WBootstrapTheme();

  virtual ~WBootstrapTheme();

  /*! Enables responsive features.
   *
   * Responsive features can be enabled only at application
   * startup. For bootstrap 3, you need to use the progressive
   * bootstrap feature of %Wt
   * \if cpp
   * (see \ref config_general)
   * \endif
   * as it requires setting HTML meta flags.
   *
   * Responsive features are disabled by default.
   */
  void setResponsive(bool responsive);

  /*! \brief Returns whether responsive features are enabled.
   *
   * \sa setResponsive()
   */
  bool responsive() const { return responsive_; }

  /*! \brief Sets the bootstrap version.
   *
   * The default bootstrap version is 2 (but this may change in the
   * future and thus we recommend setting the version).
   *
   * Since Twitter Bootstrap breaks its API with a major version
   * change, the version has a big impact on how how the markup is
   * done for various widgets.
   *
   * Note that the two Bootstrap versions have a different license:
   * Apache 2.0 for Bootstrap version 2.2.2, and MIT for version
   * 3.1. See these licenses for details.
   */
  void setVersion(BootstrapVersion version);

  /*! \brief Returns the bootstrap version.
   *
   * \sa setVersion()
   */
  BootstrapVersion version() const { return version_; }

  /*! \brief Enables form-control on all applicable form widgets.
   *
   * This is relevant only for bootstrap 3.
   *
   * By applying "form-control" on form widgets, they will become
   * block level elements that take the size of the parent (which is
   * in bootstrap's philosphy a grid layout).
   *
   * The default value is \c true.
   */
  void setFormControlStyleEnabled(bool enabled);

  virtual std::string name() const override;
  virtual std::vector<WLinkedCssStyleSheet> styleSheets() const override;
  virtual void apply(WWidget *widget, WWidget *child, int widgetRole)
    const override;
  virtual void apply(WWidget *widget, DomElement& element, int elementRole)
    const override;
  virtual std::string disabledClass() const override;
  virtual std::string activeClass() const override;
  virtual std::string utilityCssClass(int utilityCssClassRole) const override;
  virtual bool canStyleAnchorAsButton() const override;
  virtual void applyValidationStyle(WWidget *widget,
				    const Wt::WValidator::Result& validation,
				    WFlags<ValidationStyleFlag> styles)
    const override;
  virtual bool canBorderBoxElement(const DomElement& element) const override;

private:
  BootstrapVersion version_;
  bool responsive_, formControlStyle_;

  std::string classBtn(WWidget *widget) const;
  std::string classBar() const;
  std::string classAccordion() const;
  std::string classAccordionGroup() const;
  std::string classAccordionHeading() const;
  std::string classAccordionBody() const;
  std::string classAccordionInner() const;
  std::string classNavCollapse() const;
  std::string classNavbar() const;
  std::string classBrand() const;
  std::string classNavbarSearch() const;
  std::string classNavbarLeft() const;
  std::string classNavbarRight() const;
  std::string classNavbarMenu() const;
  std::string classNavbarBtn() const;
  bool hasButtonStyleClass(WWidget *widget) const;

};

}

#endif // WT_WBOOTSTRAP_THEME_H
