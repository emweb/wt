// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef JAVASCRIPTEXAMPLE_H_
#define JAVASCRIPTEXAMPLE_H_

#include <Wt/WApplication.h>

using namespace Wt;

/**
 * \defgroup javascript Javascript - Wt interaction example
 */
/*@{*/

class Popup;

/*! \brief An example showing how to interact custom JavaScript with Wt
 *         stuff
 */
class JavascriptExample : public WApplication
{
public:
  /*! \brief Create the example application.
   */
  JavascriptExample(const WEnvironment& env);

private:
  /*! \brief The user has confirmed the payment.
   */
  void confirmed();

  /*! \brief Set the amount to be payed.
   */
  void setAmount(std::string amount);

  /*! \brief Popup for changing the amount.
   */
  std::unique_ptr<Popup> promptAmount_;

  /*! \brief Popup for paying.
   */
  std::unique_ptr<Popup> confirmPay_;

  /*! \brief WText for showing the current amount.
   */
  WText *currentAmount_;
};

/*@}*/

#endif // JAVASCRIPTEXAMPLE_H_
