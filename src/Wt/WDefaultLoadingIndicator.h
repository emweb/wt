// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WDEFAULT_LOADING_INDICATOR_H_
#define WDEFAULT_LOADING_INDICATOR_H_

#include <Wt/WText.h>
#include <Wt/WLoadingIndicator.h>

namespace Wt {

/*! \class WDefaultLoadingIndicator Wt/WDefaultLoadingIndicator.h Wt/WDefaultLoadingIndicator.h
 *  \brief A default loading indicator.
 *
 * The default loading indicator displays the text message <span
 * style="background-color: red; color: white; font-family:
 * Arial,Helvetica,sans-serif; font-size: small;">Loading...</span> in
 * the right top corner of the window.
 *
 * <h3>CSS</h3>
 *
 * This widget does not provide styling, 
 * and can be styled using inline or external CSS as appropriate.
 *
 * <h3>i18n</h3>
 *
 * The strings used in this class can be translated by overriding
 * the default values for the following localization keys:
 * - Wt.WDefaultLoadingIndicator.Loading: Loading...
 *
 * \sa WApplication::setLoadingIndicator()
 */
class WT_API WDefaultLoadingIndicator : public WLoadingIndicator
{
public:
  /*! \brief Constructor.
   */
  WDefaultLoadingIndicator();

  virtual WWidget *widget() { return this; }
  virtual void setMessage(const WString& text) override;
};

}

#endif // WDEFAULT_LOADING_INDICATOR_H_
