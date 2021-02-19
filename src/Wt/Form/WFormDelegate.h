// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2021 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_FORM_FORM_DELEGATE_H_
#define WT_FORM_FORM_DELEGATE_H_

#include <Wt/Form/WAbstractFormDelegate.h>

namespace Wt {
  namespace Form {

template<typename T, class Enable = void>
class WFormDelegate;

/*! \brief Form delegate class for WString
 *
 * This will create a WLineEdit to display the WString value
 * in the View
 */
template<>
class WT_API WFormDelegate<Wt::WString, void> : public WAbstractFormDelegate
{
public:
  /*! \brief Create a form delegate
   */
  WFormDelegate();

  /*! \brief Create WLineEdit to be used in the View
   */
  std::unique_ptr<Wt::WWidget> createFormWidget() override;
};
  }
}

#endif // WT_FORM_FORM_DELEGATE
