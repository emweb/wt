/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 */

#include <Wt/Payment/PayPal.h>
#include <Wt/Payment/Customer.h>
#include <Wt/Payment/Order.h>
#include <Wt/Payment/Money.h>

#include <Wt/WEnvironment.h>
#include <Wt/WLogger.h>
#include <Wt/WResource.h>
#include <Wt/WServer.h>
#include <Wt/WString.h>
#include <Wt/WStringStream.h>
#include <Wt/Http/Client.h>
#include <Wt/Http/Response.h>
#include <Wt/Http/HttpUtils.h>
#include <Wt/Utils.h>

#include <string>
#include <sstream>
#include <iostream>
#include <map>

#include "Wt/PopupWindow.h"

#define ERROR_MSG(e) Wt::WString::tr("Wt.Payment.PayPal." e)

namespace {
  const char *PayPalServiceUser = "paypal-user";
  const char *PayPalServicePassword = "paypal-password";
  const char *PayPalServiceSignature = "paypal-signature";
  const char *PayPalServiceApiServerUrl = "paypal-api-server-url";
  const char *PayPalServicePayServerUrl = "paypal-pay-server-url";
  const char *PayPalServiceUserVersion = "paypal-version";
}
namespace Wt {

LOGGER("Payment.PayPal");

  namespace Payment {

class PayPalRedirectResource final : public Wt::WResource
{
public:
  PayPalRedirectResource(PayPalExpressCheckout *checkout)
    : WResource(),
      checkout_(checkout)
  { }

  virtual ~PayPalRedirectResource()
  {
    beingDeleted();
  }

  virtual void handleRequest(const Wt::Http::Request& request,
                             Wt::Http::Response& response) override
  {
#ifndef WT_TARGET_JAVA
    std::ostream& o = response.out();
#else
    std::ostream o(response.out());
#endif // WT_TARGET_JAVA

    const std::string *result = request.getParameter("result");
    const std::string *payerId = request.getParameter("PayerID");

    if (result && *result == "ok" && payerId)
      checkout_->setPaymentAccepted(true, *payerId);
    else
      checkout_->setPaymentAccepted(false, std::string());

    /*
     * Parse request parameters here and set them in the checkout_
     *   we can already make the call for the details while the window
     *   is being closed ?
     */

    WApplication *app = WApplication::instance();
    std::string appJs = app->javaScriptClass();

    o <<
      "<!DOCTYPE html>"
      "<html lang=\"en\" dir=\"ltr\">\n"
      "<head><title></title>\n"
      "<script type=\"text/javascript\">\n"
      "function load() { "
      """if (window.opener." << appJs << ") {"
      ""  "var " << appJs << "= window.opener." << appJs << ";"
      <<  checkout_->redirected().createCall({"-1"}) << ";"
      ""  "window.closedAfterRedirect=true;"
      ""  "window.close();"
      "}\n"
      "}\n"
      "</script></head>"
      "<body onload=\"load();\"></body></html>";
  }

private:
  PayPalExpressCheckout *checkout_;
};

//Impl

struct PayPalExpressCheckout::Impl
{
  Impl(PayPalService& service, PayPalExpressCheckout *checkout):
    service_(service),
    usePopup_(true),
    accepted_(false),
    redirected_(checkout, "redirected"),
    paymentAction_(PaymentAction::Sale)
  { }

  PayPalService& service_;
  bool usePopup_, accepted_;

  Signal<Result> setupSignal_;
  Signal<Result> updatedCustomerDetailsSignal_;
  Signal<Approval> paymentApprovedSignal_;
  Signal<Result> completePaymentSignal_;

  JSignal<int> redirected_;
  Customer customer_;
  Order order_;
  std::unique_ptr<Http::Client> httpClient_;
  std::map<std::string, std::string> editedParameters_;

  PaymentAction paymentAction_;

  std::unique_ptr<PayPalRedirectResource> redirectResource_;
  std::string token_;
  std::string startInternalPath_;

  std::map<std::string, std::string> lastSentMessage_;
};

// PayPalExpressCheckout:

PayPalExpressCheckout::~PayPalExpressCheckout()
{
  delete impl_;
}

//private constructor:
PayPalExpressCheckout::PayPalExpressCheckout(
  PayPalService &service, const Customer &customer,  const Order &order)
{
  impl_ = new Impl(service, this);
  impl_->paymentAction_ = PaymentAction::Sale;
  impl_->order_ = order;
  impl_->customer_ = customer;
  impl_->redirectResource_.reset(new PayPalRedirectResource(this));

  impl_->usePopup_ = WApplication::instance()->environment().ajax();

  impl_->redirected_.connect(this, &PayPalExpressCheckout::onRedirect);
}

const Customer& PayPalExpressCheckout::customer() const
{
  return impl_->customer_;
}

const Order& PayPalExpressCheckout::order() const
{
  return impl_->order_;
}

void PayPalExpressCheckout::setPaymentAction(PaymentAction action)
{
  impl_->paymentAction_ = action;
}

PaymentAction
PayPalExpressCheckout::paymentAction() const
{
  return impl_->paymentAction_;
}

void PayPalExpressCheckout::setParameter( const std::string& name,
                                          const std::string& value)
{
  impl_->editedParameters_[name] = value;
}

Signal<Result>& PayPalExpressCheckout::setup()
{
  impl_->httpClient_ = impl_->service_.createHttpClient();
  auto client = impl_->httpClient_.get();

  client->done().connect
    (this, std::bind(&PayPalExpressCheckout::handleSetup, this,
		     std::placeholders::_1,
		     std::placeholders::_2));

  std::map<std::string, std::string> map;
  createSetupMessage(map);
  addEditedParameters(map);
  std::string messageText = encodeMessage(map);

  LOG_DEBUG("message: " << messageText);
  LOG_DEBUG("message: " << Wt::Utils::urlDecode(messageText));

  Http::Message message;
  message.addBodyText(messageText);

  impl_->lastSentMessage_ = map;

  if (!client->post(impl_->service_.apiServerUrl(), message))
    LOG_ERROR("error submiting POST");

  return impl_->setupSignal_;
}

std::string
PayPalExpressCheckout::encodeMessage(const std::map<std::string, std::string>&
				     map) const
{
  WStringStream ans;

  for (StringMap::const_iterator i = map.begin(); i != map.end(); ++i)
    ans << "&" << i->first << "=" << Wt::Utils::urlEncode(i->second);
  
  return ans.str();
}

void PayPalExpressCheckout::addUserFields(std::map<std::string, std::string> &map)
{
  map["USER"] = impl_->service_.user();
  map["PWD"] = impl_->service_.password();
  map["SIGNATURE"] = impl_->service_.signature();
  map["VERSION"] = impl_->service_.version();
}

void PayPalExpressCheckout::addEditedParameters(
  std::map<std::string, std::string> &map)
{
  for(StringMap::iterator i = impl_->editedParameters_.begin();
      i != impl_->editedParameters_.end(); ++i) {
    map[i->first] = i->second;
  }
}


void PayPalExpressCheckout::createSetupMessage(
  std::map<std::string, std::string> &map)
{
  std::string currency = impl_->order_.totalOrderCost().currency();

  addUserFields(map);

  const Order& order = impl_->order_;

  map["PAYMENTREQUEST_0_PAYMENTACTION"] = toString(impl_->paymentAction_);
  map["PAYMENTREQUEST_0_AMT"] = order.totalOrderCost().toString();
  map["PAYMENTREQUEST_0_CURRENCYCODE"] = currency;
  map["RETURNURL"] = returnUrl();
  map["CANCELURL"] = cancelUrl();
  map["METHOD"] = "SetExpressCheckout";

  // Order info:
  map["PAYMENTREQUEST_0_DESC"] = order.description();
  map["PAYMENTREQUEST_0_ITEMAMT"] = order.totalItemCost().toString();
  map["PAYMENTREQUEST_0_TAXAMT"] = order.tax().toString();
  map["PAYMENTREQUEST_0_SHIPPINGAMT"] = order.shipping().toString();
  map["LOCALECODE"] = impl_->customer_.locale();
  map["PAYMENTREQUEST_0_HANDLINGAMT"] = order.handling().toString();
  map["PAYMENTREQUEST_0_SHIPDISCAMT"] = order.shippingDiscount().toString();
  map["PAYMENTREQUEST_0_INSURANCEAMT"] = order.shippingInsurance().toString();

  // Item info:
  WStringStream text;
  for (unsigned m = 0; m < order.items().size(); ++m){
    const OrderItem &item = order.items()[m];
    text.clear();
    text << "L_PAYMENTREQUEST_0_NAME" << (int)m;
    map[text.str()] = item.name().toUTF8();

    text.clear();
    text << "L_PAYMENTREQUEST_0_DESC" << (int)m;
    map[text.str()] = item.description().toUTF8();

    text.clear();
    text << "L_PAYMENTREQUEST_0_NUMBER" << (int)m;
    map[text.str()] = item.number();

    text.clear();
    text << "L_PAYMENTREQUEST_0_AMT" << (int)m;
    map[text.str()] = item.unitCost().toString();

    text.clear();
    text << "L_PAYMENTREQUEST_0_QTY" << (int)m;
    std::string quantity = std::to_string(item.quantity());
    map[text.str()] = quantity;
  }

  // Customer info:
  const Customer& customer = impl_->customer_;

  map["EMAIL"] = customer.email();
  map["FIRSTNAME"] = customer.firstName().toUTF8();
  map["LASTNAME"] = customer.lastName().toUTF8();
  map["PAYMENTREQUEST_0_SHIPTOCITY"]
    = customer.shippingAddress().city().toUTF8();
  map["PAYMENTREQUEST_0_SHIPTOCOUNTRYCODE"]
    = customer.shippingAddress().countryCode();
  map["PAYMENTREQUEST_0_SHIPTONAME"]
    = customer.shippingAddress().name().toUTF8();
  map["PAYMENTREQUEST_0_SHIPTOPHONRNUM"]
    = customer.shippingAddress().phoneNumber();
  map["PAYMENTREQUEST_0_SHIPTOSTATE"]
    = customer.shippingAddress().state().toUTF8();
  map["PAYMENTREQUEST_0_SHIPTOSTREET"]
    = customer.shippingAddress().street1().toUTF8();
  map["PAYMENTREQUEST_0_SHIPTOSTREET2"]
    = customer.shippingAddress().street2().toUTF8();
  map["PAYMENTREQUEST_0_SHIPTOZIP"]
    = customer.shippingAddress().zip().toUTF8();

}

std::map<std::string, std::string>
PayPalExpressCheckout::parametersMapToMap(Http::ParameterMap &map)
{
  std::map<std::string, std::string> ans;

  for (Http::ParameterMap::iterator i  = map.begin(); i != map.end(); ++i)
    ans[i->first] = i->second[0];

  return ans;
}

std::string PayPalExpressCheckout::toString(PaymentAction action)
{
  switch (action) {
  case PaymentAction::Sale:          return "Sale";
  case PaymentAction::Authorization: return "Authorization";
  case PaymentAction::Order:         return "Order";
  default:
    throw WException("Unknown payment action");
  }
}


void PayPalExpressCheckout::handleSetup(AsioWrapper::error_code err,
					const Http::Message& response)
{
  Result result;

  LOG_DEBUG("handleSetup::Received response: "
            << Wt::Utils::urlDecode(response.body()));

  result = testMessage(err, response);

  impl_->setupSignal_.emit(result);
}

void PayPalExpressCheckout::setToken(const std::string& token)
{
  impl_->token_ = token;

  if (impl_->usePopup_) {
    WApplication *app = WApplication::instance();

    PopupWindow::loadJavaScript(app);

    WStringStream js;

    js << WT_CLASS ".PopupWindow("
       << WT_CLASS << ","
       << WWebWidget::jsStringLiteral(paymentUrl()) << ","
       << impl_->service_.popupWidth() << ","
       << impl_->service_.popupHeight() << ","
       << "function(w) { if (!w.closedAfterRedirect) {"
       << ""          <<  impl_->redirected_.createCall({"2"})
       << "} });";

#ifndef WT_TARGET_JAVA
    implementJavaScript(&PayPalExpressCheckout::startPayment, js.str());
#endif
  }
}

std::string PayPalExpressCheckout::paymentUrl() const
{
  WStringStream url;

  url << impl_->service_.payServerUrl()
      << "?cmd=_express-checkout&token="
      << Wt::Utils::urlEncode(impl_->token_);

  return url.str();
}

void PayPalExpressCheckout::startPayment()
{
  if (!impl_->usePopup_) {
    WApplication *app = WApplication::instance();
    impl_->startInternalPath_ = app->internalPath();
    app->redirect(paymentUrl());
  }
}

void PayPalExpressCheckout::setPaymentAccepted(bool accepted,
					       const std::string& payerId)
{
  impl_->accepted_ = accepted;
  impl_->customer_.setPayerId(payerId);
}

void PayPalExpressCheckout::onRedirect(int result)
{
  ApprovalOutcome outcome;

  if (result == -1) {
    outcome = impl_->accepted_ 
      ? ApprovalOutcome::Accepted : ApprovalOutcome::Denied;
  } else {
    outcome = ApprovalOutcome::Interrupted;
  }

  paymentApproved().emit(Approval(outcome));
}

Signal<Result>& PayPalExpressCheckout::updateCustomerDetails()
{
  impl_->httpClient_ = impl_->service_.createHttpClient();
  auto client = impl_->httpClient_.get();

  client->done().connect
    (this, std::bind(&PayPalExpressCheckout::handleCustomerDetails, this,
		     std::placeholders::_1,
		     std::placeholders::_2));

  std::map<std::string, std::string> map;

  addUserFields(map);
  map["TOKEN"] = impl_->token_;
  map["METHOD"] = "GetExpressCheckoutDetails";

  addEditedParameters(map);
  std::string msg = encodeMessage(map);

  LOG_DEBUG("message: " << msg);
  LOG_DEBUG("message: " << Wt::Utils::urlDecode(msg));

  Http::Message message;
  message.addBodyText(msg);
  impl_->lastSentMessage_ = map;


  if (!client->post(impl_->service_.apiServerUrl(), message))
    LOG_ERROR("error submiting POST");

  return impl_->updatedCustomerDetailsSignal_;
}

void PayPalExpressCheckout::handleCustomerDetails(
  AsioWrapper::error_code err,
  const Http::Message& response)
{
  Result result;
  Http::ParameterMap params;
  Http::Utils::parseFormUrlEncoded(response, params);

  LOG_DEBUG("handleCustomerDetails - Received response: "
            << Wt::Utils::urlDecode(response.body()));

  result = testMessage(err, response);
  if(!result.error()){
    saveCustomerDetails(params);
  }

  impl_->updatedCustomerDetailsSignal_.emit(result);
}

std::string PayPalExpressCheckout::prameterValue(
  const std::string &parameterName, Http::ParameterMap &params)
{
  Http::ParameterMap::iterator it;
  it = params.find(parameterName);
  if(it != params.end())
    return it->second[0];

  return "";
}

void PayPalExpressCheckout::saveCustomerDetails(Http::ParameterMap &params)
{
  //save contents.
  impl_->customer_.setEmail(prameterValue("EMAIL", params));
  impl_->customer_.setFirstName(prameterValue("FIRSTNAME", params));
  impl_->customer_.setLastName(prameterValue("LASTNAME", params));
  impl_->customer_.setLocale(prameterValue("LOCALECODE", params));
  impl_->customer_.setPayerId(prameterValue("PAYERID", params));

  //Save address
  Address adderss = impl_->customer_.shippingAddress();

  adderss.setCity(prameterValue("PAYMENTREQUEST_0_SHIPTOCITY", params));
  adderss.setCountryCode(prameterValue("PAYMENTREQUEST_0_SHIPTOCOUNTRYCODE",
                         params));
  adderss.setName(prameterValue("PAYMENTREQUEST_0_SHIPTONAME", params));
  adderss.setPhoneNumber(prameterValue("PAYMENTREQUEST_0_SHIPTOPHONRNUM",
                         params));
  adderss.setState(prameterValue("PAYMENTREQUEST_0_SHIPTOSTATE", params));
  adderss.setStreet1(prameterValue("PAYMENTREQUEST_0_SHIPTOSTREET", params));
  adderss.setStreet2(prameterValue("PAYMENTREQUEST_0_SHIPTOSTREET2", params));
  adderss.setZip(prameterValue("PAYMENTREQUEST_0_SHIPTOZIP", params));

  impl_->customer_.setShippingAddress(adderss);
}

Signal<Result>& PayPalExpressCheckout::completePayment(const Money& totalAmount)
{
  impl_->httpClient_ = impl_->service_.createHttpClient();
  auto client = impl_->httpClient_.get();

  client->done().connect
    (this, std::bind(&PayPalExpressCheckout::handleCompletePayment, this,
		     std::placeholders::_1,
		     std::placeholders::_2));

  std::map<std::string, std::string> map;

  addUserFields(map);

  map["PAYMENTREQUEST_0_PAYMENTACTION"] = toString(impl_->paymentAction_);
  map["PAYMENTREQUEST_0_AMT"] = impl_->order_.totalOrderCost().toString();
  map["PAYMENTREQUEST_0_CURRENCYCODE"] = impl_->order_.totalOrderCost()
    .currency();
  map["PAYERID"] = impl_->customer_.payerId();
  map["TOKEN"] = impl_->token_;
  map["METHOD"] = "DoExpressCheckoutPayment";

  addEditedParameters(map);
  std::string msg = encodeMessage(map);
  LOG_DEBUG("message: " << msg);
  LOG_DEBUG("message: " << Wt::Utils::urlDecode(msg));

  Http::Message message;
  message.addBodyText(msg);

  impl_->lastSentMessage_ = map;

  if (!client->post(impl_->service_.apiServerUrl(), message))
    LOG_ERROR("error submiting POST");

  return impl_->completePaymentSignal_;
}

void PayPalExpressCheckout::handleCompletePayment(
  AsioWrapper::error_code err, const Http::Message& response)
{

  LOG_DEBUG("handleCompletePayment ::Received response: "
            << Wt::Utils::urlDecode(response.body()));

  Http::ParameterMap params;
  Http::Utils::parseFormUrlEncoded(response, params);

  std::cout<< messageToString(params);

  Result result = testMessage(err, response);
  impl_->completePaymentSignal_.emit(result);
}


Signal<Approval>& PayPalExpressCheckout::paymentApproved()
{
  return impl_->paymentApprovedSignal_;
}

std::string PayPalExpressCheckout::cancelUrl() const
{
  return WApplication::instance()->makeAbsoluteUrl
    (impl_->redirectResource_->url() + "&result=fail");
}

std::string PayPalExpressCheckout::returnUrl() const
{
  return WApplication::instance()->makeAbsoluteUrl
    (impl_->redirectResource_->url() + "&result=ok");
}

JSignal<int>& PayPalExpressCheckout::redirected()
{
  return impl_->redirected_;
}

Result PayPalExpressCheckout::testMessage(AsioWrapper::error_code err,
                   const Http::Message& response)
{
  Result result;
  Http::ParameterMap params;
  Http::Utils::parseFormUrlEncoded(response, params);

  if (!err) {
    if (response.status() == 200) {
      const std::string *ack = Http::Utils::getParamValue(params, "ACK");

      if (ack) {
        LOG_DEBUG("ACK = " << *ack);
        if (*ack == "Success") {
          const std::string *token 
	    = Http::Utils::getParamValue(params, "TOKEN");

          if (token) {
            setToken(*token);
          } else {
            result = Result(ERROR_MSG("missing-token"));
          }
        } else {
          const std::string *errorCode 
	    = Http::Utils::getParamValue(params, "ERRORCODE0");

          if (errorCode) {
            result = Result(ERROR_MSG("error").arg(*errorCode));
          } else {
            result = Result(ERROR_MSG("bad-ack").arg(*ack));
          }
        }
      } else {
        result = Result(ERROR_MSG("missing-ack"));
      }
    } else {
      result = Result(ERROR_MSG("error"));
    }
  } else {
    result = Result(WString::fromUTF8(err.message()));
  }

  result.setResponseMessage(parametersMapToMap(params));
  result.setRequestMessage(impl_->lastSentMessage_);
  return result;
}

void PayPalExpressCheckout::printMessage(Http::ParameterMap &params) const
{
  std::cout << messageToString(params);
}

std::string PayPalExpressCheckout::messageToString(Http::ParameterMap &params)
  const
{
  WStringStream ans;

  for(Http::ParameterMap::const_iterator i = params.begin(); 
      i != params.end(); ++i){
    const Http::ParameterValues &val = i->second;
    ans << i->first << " : " << val[0] << " \n ";
  }

  return ans.str();
}

PayPalService::PayPalService()
{

}

PayPalService::~PayPalService()
{

}

bool PayPalService::configureFromProperties()
{
  try {
    user_ = configurationProperty(PayPalServiceUser);
    password_ = configurationProperty(PayPalServicePassword);
    signature_ = configurationProperty(PayPalServiceSignature);
    apiServerUrl_ = configurationProperty(PayPalServiceApiServerUrl);
    payServerUrl_ = configurationProperty(PayPalServicePayServerUrl);
    version_ = configurationProperty(PayPalServiceUserVersion);

    return true;
  } catch (const std::exception& e) {
    LOG_INFO("not configured: " << e.what());

    return false;
  }
}

std::string PayPalService::configurationProperty(
  const std::string& property)
{
  WServer *instance = WServer::instance(); // Xx hmmmm...

  if (instance) {
    std::string result;

      bool error;
#ifndef WT_TARGET_JAVA
      error = !instance->readConfigurationProperty(property, result);
#else
      std::string* v = instance->readConfigurationProperty(property, result);
      if (v != &result) {
        error = false;
        result = *v;
      } else {
        error = true;
      }
#endif

    if (error)
      throw WException("PayPalService: no '" + property +
                       "' property configured");

    return result;
  } else
    throw WException(
        "PayPalService: could not find a WServer instance");
}

void PayPalService::configureTestSandbox()
{
  user_ = "sdk-three_api1.sdk.com";
  password_ = "QFZCWN5HZM8VBG7Q";
  signature_ =
        "A-IzJhZZjhg29XQ2qnhapuwxIDzyAZQ92FRP5dqBzVesOkzbdUONzmOU";
  apiServerUrl_ = "https://api-3t.sandbox.paypal.com/nvp";
  payServerUrl_ = "https://www.sandbox.paypal.com/webscr";
  version_ = "92.0";
}

void PayPalService::setUser(const std::string& user)
{
  user_ = user;
}

void PayPalService::setPassword(const std::string& password)
{
  password_ = password;
}

void PayPalService::setSignature(const std::string& signature)
{
  signature_ = signature;
}

void PayPalService::setVersion(const std::string& version)
{
  version_ = version;
}

void PayPalService::setApiServerUrl(const std::string& url)
{
  apiServerUrl_ = url;
}

void PayPalService::setPayServerUrl(const std::string& url)
{
  payServerUrl_ = url;
}

int PayPalService::popupWidth() const
{
  return 980;
}

int PayPalService::popupHeight() const
{
  return 600;
}

std::unique_ptr<Http::Client> PayPalService::createHttpClient()
{
  std::unique_ptr<Http::Client> result(new Http::Client());
  result->setTimeout(std::chrono::seconds{15});
  return result;
}

PayPalExpressCheckout *
PayPalService::createExpressCheckout(
  const Customer& customer, const Order& order)
{
  return new PayPalExpressCheckout(*this, customer, order);
}

  }
}
