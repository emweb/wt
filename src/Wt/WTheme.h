// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2012 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_WTHEME_H
#define WT_WTHEME_H

#include <Wt/WObject.h>
#include <Wt/WGlobal.h>
#include <Wt/WValidator.h>

namespace Wt {

class DomElement;
class WStringStream;

/*! \brief Enumeration for the role of a subwidget (for theme support)
 *
 * \sa WTheme::apply()
 */
enum WidgetThemeRole {
  MenuItemIcon = 100,
  MenuItemCheckBox = 101,
  MenuItemClose = 102,

  DialogCoverWidget = 200,
  DialogTitleBar = 201,
  DialogBody = 202,
  DialogFooter = 203,
  DialogCloseIcon = 204,
  DialogContent = 205, // For bootstrap 3 modal-content style

  TableViewRowContainer = 300,

  DatePickerPopup = 400,
  TimePickerPopup = 410,

  PanelTitleBar = 500,
  PanelCollapseButton = 501,
  PanelTitle = 502,
  PanelBody = 503,

  AuthWidgets = 600,

  InPlaceEditing = 700,

  Navbar = 800,
  NavCollapse = 801,
  NavBrand = 802,
  NavbarSearch = 803,
  NavbarMenu = 804,
  NavbarBtn = 805,
  NavbarAlignLeft = 806,
  NavbarAlignRight = 807
};

/*! \brief Enumeration for the role of a css class (for theme support)
 *
 * \sa WTheme::utilityCssClass()
 */
enum UtilityCssClassRole {
  ToolTipInner = 100,
  ToolTipOuter = 101
};

/*! \brief Enumeration for the role of a DOM element (for theme support)
 *
 * \sa WTheme::apply()
 */
enum ElementThemeRole {
  MainElement = 0,
  ProgressBarBar = 100,
  ProgressBarLabel = 101
};

/*! \brief Enumeration that indicates what validation styles are to be applie
 *
 * \sa WTheme::applyValidationStyle()
 */
enum class ValidationStyleFlag {
  InvalidStyle = 0x1,
  ValidStyle = 0x2
};

W_DECLARE_OPERATORS_FOR_FLAGS(ValidationStyleFlag)

/** \brief All validation styles */
static const WFlags<ValidationStyleFlag> ValidationAllStyles
  = ValidationStyleFlag::InvalidStyle | ValidationStyleFlag::ValidStyle;

/*! \class WTheme Wt/WTheme.h Wt/WTheme.h
 *
 * Abstract base class for themes in %Wt.
 *
 * \sa WApplication::setTheme()
 */
class WT_API WTheme : public WObject
{
public:
  /*! \brief Constructor.
   */
  WTheme();

  /*! \brief Destructor.
   */
  virtual ~WTheme();

  /*! \brief Returns a theme name.
   *
   * Returns a unique name for the theme. This name is used by the default
   * implementation of resourcesUrl() to compute a location for the theme's
   * resources.
   */
  virtual std::string name() const = 0;

  /*! \brief Returns the URL where theme-related resources are stored.
   *
   * The default implementation considers a folder within %Wt's
   * resource directory, based on the theme name().
   */
  virtual std::string resourcesUrl() const;

  /*! \brief Serves the CSS for the theme.
   *
   * This must serve CSS declarations for the theme.
   *
   * The default implementation serves all the styleSheets().
   */
  virtual void serveCss(WStringStream& out) const;

  /*! \brief Returns a vector with stylesheets for the theme.
   *
   * This should return a vector with stylesheets that implement the
   * theme. This list may be tailored to the current user agent, which
   * is read from the application environment.
   */
  virtual std::vector<WLinkedCssStyleSheet> styleSheets() const = 0;

  /*! \brief Applies the theme to a child of a composite widget.
   *
   * The \p widgetRole indicates the role that \p child has within the
   * implementation of the \p widget.
   */
  virtual void apply(WWidget *widget, WWidget *child, int widgetRole)
    const = 0;

  /*! \brief Applies the theme to a DOM element that renders a widget.
   *
   * The \p element is a rendered representation of the \p widget, and
   * may be further customized to reflect the theme.
   */
  virtual void apply(WWidget *widget, DomElement& element, int elementRole)
    const = 0;

  /*! \brief Returns a generic CSS class name for a disabled element.
   */
  virtual std::string disabledClass() const = 0;

  /*! \brief Returns a generic CSS class name for an active element.
   */
  virtual std::string activeClass() const = 0;

  /*! \brief Returns a generic CSS class name for the chosen role.
   *
   * \sa WTheme::utilityCssClassRole
   */
  virtual std::string utilityCssClass(int utilityCssClassRole) const = 0;

  /*! \brief Returns whether the theme allows for an anchor to be styled
   *         as a button.
   */
  virtual bool canStyleAnchorAsButton() const = 0;

  /*! \brief Applies a style that indicates the result of validation.
   */
  virtual void applyValidationStyle(WWidget *widget,
                    const Wt::WValidator::Result& validation,
                    WFlags<ValidationStyleFlag> flags) const
    = 0;

  virtual bool canBorderBoxElement(const DomElement& element) const = 0;
};

}

#endif // WT_WTHEME_H
