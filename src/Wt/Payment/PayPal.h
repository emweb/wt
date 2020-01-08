// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2012 Emweb bv, Herent, Belgium.
 */
#ifndef WT_PAYMENT_PAYPAL_H
#define WT_PAYMENT_PAYPAL_H

#include <Wt/WObject.h>
#include <Wt/WSignal.h>
#include <Wt/WJavaScript.h>
#include <Wt/Payment/Result.h>

#include <Wt/AsioWrapper/system_error.hpp>

#include <map>
namespace Wt {
  namespace Http{
  }

  /*! \brief Namespace for the \ref Payment
   */
  namespace Payment {

/*! \brief Enumeration for payment actions.
 *
 * See more information at PayPal:
 * 
 * https://cms.paypal.com/us/cgi-bin/?cmd=_render-content&content_ID=developer/e_howto_api_nvp_r_SetExpressCheckout
 *
 * \sa setPaymentAction()
 */
enum class PaymentAction {
  Sale,          //!< A Sale action
  Authorization, //!< An Authorization action
  Order          //!< An Order action
};

class Customer;
class Money;
class Order;
class PayPalService;

/*! \class PayPalExpressCheckout Wt/Payment/PayPal.h Wt/Payment/Paypal
 *  \brief A paypal express checkout process.
 *
 * This implements the PayPal Express Checkout process.
 *
 * For more information about the PayPal API see:
 * https://cms.paypal.com/us/cgi-bin/?cmd=_render-content&content_ID=developer/e_howto_api_ECGettingStarted
 *
 * Use flow:
 * -# use PayPalService to create a PayPalExpressCheckout instance.
 * -# setup() - sends SetExpressCheckout API call returns a result signal.
 * -# startPayment() -  Redirects to PayPal. Use it after receiving the setup()
 *    result signal.
 * -# paymentApproved signal will be emitted with the result of
 *    SetExpressCheckout API call.
 * -# updateCustomerDetails() - GetExpressCheckoutDetails API call returns a
 *    result signal.
 * -# completePayment() - DoExpressCheckoutPayment API call returns a result
 *    signal.
 *
 * For an example, see the paypal feature example. This example reads the necessary variables from the wt_config.xml file.
 *
 * For the api server url you should use API signature with the SOAP format. More information about the necessary url's and SOAP: https://developer.paypal.com/docs/classic/api/endpoints/ and https://developer.paypal.com/docs/integration/direct/identity/seamless-checkout/#nvp-soap-credentials.
 *
 * The login credentials can be found in a paypal business account. You can create a sandbox test account by following the documentantion: https://developer.paypal.com/docs/classic/lifecycle/sb_create-accounts.
 *
 * \sa PayPalService
 *
 * \ingroup payment
 */
class WT_API PayPalExpressCheckout : public WObject
{
public:
  virtual ~PayPalExpressCheckout();

  /*! \brief Sets the payment action
   *
   * The default payment action is SaleAction.
   */
  void setPaymentAction(PaymentAction action);

  /*! \brief Returns the payment action.
   *
   * \sa setPaymentAction()
   */
  PaymentAction paymentAction() const;

  /*! \brief Adds or overrides a PayPal API request parameter.
   *
   * If value is empty, then this will remove a request parameter.
   *
   * Useful use examples:
   * \code
   *  //send to paypal - &REQCONFIRMSHIPPING=1
   *  setParameter("REQCONFIRMSHIPPING", 1);
   *
   * //send to paypal - &NOSHIPPING=1
   *  setParameter("NOSHIPPING", 1);
   *
   * //send to paypal - &LOGOIMG=https://www.../logo.gif
   *  setParameter("LOGOIMG", "https://www.../logo.gif");
   *
   * //send to paypal - &CARTBORDERCOLOR=00CD
   *  setParameter("CARTBORDERCOLOR", "00CD");
   * \endcode
   */
  void setParameter(const std::string& name, const std::string& value);

  /*! \brief Returns the customer information.
   *
   * \sa Customer
   */
  const Customer& customer() const;

  /*! \brief Returns the order information.
   *
   * \sa Order
   */
  const Order& order() const;

  /*! \brief Setup the transaction.
   *
   * This uses the PayPal SetExpressCheckout API call to initiate the
   * payment process, preparing things for a subsequent call to
   * startPayment().
   *
   * This is an asynchronous call, which returns a signal that is emitted
   * when the call has finished.
   */
  Signal<Result>& setup();

  /*! \brief Start the payment.
   *
   * This redirects to PayPal. It should be directly connected to a clicked
   * event (on the PayPal button), to allow a JavaScript popup window to
   * be used.
   *
   * This is an asynchronous call, whose result will be indicated
   * using the paymentApproved() signal.
   */
  void startPayment();

  /*! \brief Updates customer information from PayPal.
   *
   * This fetches customer information that is kept at PayPal for the
   * current user. This call is optional, and can be used only after
   * the user approved the payment (see paymentApproved()).
   *
   * This uses the PayPal GetExpressCheckoutDetails API call.
   *
   * This is an asynchronous call, which returns a signal that is emitted
   * when the call has finished.
   */
  Signal<Result>& updateCustomerDetails();

  /*! \brief Completes the payment.
   *
   * This is the last step of a PayPal checkout, which uses the PayPal
   * DoExpressCheckoutPayment API call to confirm the payment.
   *
   * This may update the exact amount (for example to reflect accurate
   * shipping information knowing the customer's shipping address).
   *
   * This is an asynchronous call, which returns a signal that is emitted
   * when the call has finished.
   */
  Signal<Result>& completePayment(const Money& totalAmount);

  /*! \brief The payment approval signal.
   *
   * \sa startPayment()
   */
  Signal<Approval>& paymentApproved();

private:
  struct Impl;
  Impl *impl_;

  PayPalExpressCheckout(PayPalService& service,
                        const Customer& customer, const Order& order);

  std::string encodeMessage(const std::map<std::string, std::string> &map)
    const;
  void addUserFields(std::map<std::string, std::string> &map);
  void addEditedParameters(std::map<std::string, std::string> &map);
  void createSetupMessage(std::map<std::string, std::string> &map);
  std::string toString(PaymentAction action);

  std::map<std::string, std::string> parametersMapToMap
  (Http::ParameterMap &map);

  void handleSetup(Wt::AsioWrapper::error_code err,
                   const Http::Message& response);

  void setToken(const std::string& url);
  std::string cancelUrl() const;
  std::string returnUrl() const;
  std::string paymentUrl() const;

  JSignal<int>& redirected();
  void setPaymentAccepted(bool accepted, const std::string& payerId);
  void onRedirect(int result);

  void handleInternalPath(const std::string& internalPath);
  void handleCustomerDetails(Wt::AsioWrapper::error_code err,
                   const Http::Message& response);
  std::string prameterValue(const std::string &parameterName,
                            Http::ParameterMap &params);
  void saveCustomerDetails(Http::ParameterMap &params);

  void handleCompletePayment(Wt::AsioWrapper::error_code err,
                             const Http::Message& response);

  //test the msg and saves the token.
  Result testMessage(Wt::AsioWrapper::error_code err,
                     const Http::Message& response);

  void printMessage(Http::ParameterMap &params) const;
  std::string messageToString(Http::ParameterMap &params) const;

  friend class PayPalService;
  friend class PayPalRedirectResource;

  typedef std::map<std::string, std::string> StringMap;
};


/*! \class PayPalService Wt/Payment/PayPal.h Wt/Payment/PayPal.h
 *  \brief This is a PayPal service class.
 *
 * This class holds the PayPal configuration, and is usually shared between
 * sessions.
 *
 * \ingroup payment
 */
class WT_API PayPalService
{
public:
  /*! \brief Default constructor.
   *
   * You can call configureFromProperties() to configure the service
   * from properties in the configuration, or configureTestSandbox()
   * to configure the PayPal sanbox (for testing).
   *
   * Alternatively, you could set a suitable configuration using the
   * individual methods.
   */
  PayPalService();

  /* \brief Destructor.
   */
  virtual ~PayPalService();

  /*! \brief Configures the service using properties.
   *
   * Returns \c true if values were found for all required properties:
   * - paypal-user
   * - paypal-password
   * - paypal-signature
   * - paypal-api-server-url
   * - paypal-pay-server-url
   * - paypal-version
   *
   * \note the PayPalExpressCheckout process assumes version 92.0
   */
  bool configureFromProperties();

  /*! \brief Configures the service for the PayPal test sandbox.
   */
  void configureTestSandbox();

  /*! \brief Sets the user.
   */
  void setUser(const std::string& user);

  /*! \brief Returns the user.
   *
   * \sa setUser()
   */
  std::string user() const {return user_;}

  /*! \brief Sets the password.
   */
  void setPassword(const std::string& password);

  /*! \brief Returns the password.
   *
   * \sa setPassword()
   */
  std::string password() const {return password_;}

  /*! \brief Sets the signature.
   */
  void setSignature(const std::string& signature);

  /*! \brief Returns the signature.
   *
   * \sa setSignature()
   */
  std::string signature() const {return signature_;}

  /*! \brief Sets the version
   *
   * The Paypal version that is used.
   *
   * \note the PayPalExpressCheckout process assumes version 92.0
   */
  void setVersion(const std::string& version);

  /*! \brief Returns version
   *
   * \sa setVersion()
   */
  std::string version() const {return version_;}

  /*! \brief Sets the PayPal API server url
   *
   * This is the server that is communicated with using the PayPal API.
   */
  void setApiServerUrl(const std::string& url);

  /*! \brief Returns PayPal API server url
   *
   * \sa setApiServerUrl()
   */
  std::string apiServerUrl() const {return apiServerUrl_;}

  /*! \brief Sets the payment server url.
   *
   * This is the server to which the user is redirected for the payment.
   */
  void setPayServerUrl(const std::string& url);

  /*! \brief Returns the payment server url.
   *
   * \sa setPayServerUrl()
   */
  std::string payServerUrl() const {return payServerUrl_;}  

  int popupWidth() const;
  int popupHeight() const;

  /*! \brief Starts a PayPal checkout process.
   *
   * Every distinct PayPal checkout process is managed by a
   * PayPalExpressCheckout instance, which tracks the state and
   * progress of the payment.
   */
  PayPalExpressCheckout *createExpressCheckout(const Customer& customer,
					       const Order& order);

protected:
  virtual std::unique_ptr<Http::Client> createHttpClient();

private:
  std::string user_;
  std::string password_;
  std::string signature_;
  std::string version_;
  std::string apiServerUrl_, payServerUrl_;

  std::string configurationProperty(const std::string& property);

  friend class PayPalExpressCheckout;
};

  }
}

#endif // WT_PAYMENT_PAYPAL_H
