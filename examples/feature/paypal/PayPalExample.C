/*
 * Copyright (C) 2012 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <Wt/WAnchor>
#include <Wt/WApplication>
#include <Wt/WBreak>
#include <Wt/WContainerWidget>
#include <Wt/WDialog>
#include <Wt/WGridLayout>
#include <Wt/WImage>
#include <Wt/WLabel>
#include <Wt/WMessageBox>
#include <Wt/WPushButton>
#include <Wt/WServer>
#include <Wt/WStringStream>
#include <Wt/WTable>
#include <Wt/WTemplate>
#include <Wt/WText>
#include <Wt/WVBoxLayout>
#include <Wt/Utils>

#include <Wt/Payment/PayPal>
#include <Wt/Payment/Customer>
#include <Wt/Payment/Money>
#include <Wt/Payment/Order>
#include <Wt/Payment/Address>

#include "boost/lexical_cast.hpp"

Wt::Payment::PayPalService *payPalService;
const char *PAYPAL_BUTTON_URL =
  "https://www.paypal.com/en_US/i/btn/btn_xpressCheckout.gif";

class PayPalApplication : public Wt::WApplication
{
public:
  PayPalApplication(const Wt::WEnvironment& env)
    : Wt::WApplication(env),
      expressCheckout_(0),
      payment_(0),
      dialog_(0)
  {
    messageResourceBundle().use("text");
    useStyleSheet("css/style.css");

    Wt::WPushButton *checkout = new Wt::WPushButton("Proceed to checkout...");
    checkout->clicked().connect(this, &PayPalApplication::reviewOrder);

    Wt::WTemplate *t = new Wt::WTemplate(Wt::WString::tr("start"), root());
    t->bindWidget("checkout", checkout);
  }

  ~PayPalApplication()
  {
    if (expressCheckout_)
      delete expressCheckout_;

    clearDialog();
  }

private:
  Wt::Payment::PayPalExpressCheckout* expressCheckout_;
  Wt::WContainerWidget *payment_;

  Wt::WDialog *dialog_;
  Wt::WText *dialogText_;
  Wt::WPushButton *dialogCancelButton_;

  void reviewOrder()
  {
    /*
     * First page, shows order and pay pal button
     */

    Wt::Payment::Order order = createOrder();
    Wt::Payment::Customer customer = createCustomer();

    root()->clear();
    new Wt::WText(Wt::WString::tr("review.title"), root());
    printOrder(order, true);

    expressCheckout_ = payPalService->createExpressCheckout(customer, order);

    std::string logo = "http://www.webtoolkit.eu/css/wt/emweb_powered.jpg";
    expressCheckout_->setParameter("LOGOIMG", logo);
    expressCheckout_->setParameter("CARTBORDERCOLOR", "000CD");

    expressCheckout_->setup().connect(this, &PayPalApplication::onSetup);
    enableUpdates(true);

    payment_ = new Wt::WContainerWidget(root());
    payment_->setStyleClass("payment");
    new Wt::WText("Preparing payment...", payment_);
  }

  void onSetup(const Wt::Payment::Result& result)
  {
    enableUpdates(false);
    triggerUpdate();

    if (!result.error()) {
      payment_->clear();

      // Create the PayPal button
      Wt::WAnchor *payButton = new Wt::WAnchor(payment_);
      payButton->setImage(new Wt::WImage(PAYPAL_BUTTON_URL));

      // Clicking starts the payment process
      payButton->clicked().connect
	(expressCheckout_,
	 &Wt::Payment::PayPalExpressCheckout::startPayment);

      // Clicking also shows an information dialog
      payButton->clicked().connect
	(this, &PayPalApplication::showPaymentDialog);

      // Connect paypal response(approval) from the popup window with
      // onPaymentApproval
      expressCheckout_
	->paymentApproved().connect(this,
				    &PayPalApplication::onPaymentApproval);
    } else {
      std::cerr << "result.errorMessage: "
                << result.errorMessage().toUTF8() << std::endl;

      showError("Could not setup payment with PayPal.");
    }

    printResultMessage(result);
  }

  void showPaymentDialog()
  {
    /*
     * A dialog shown while the user pays in a popup window.
     */
    enableUpdates(true);

    dialog_ = new Wt::WDialog("Information");

    dialogText_ = new Wt::WText("<p>Payment using PayPal in progress ...</p>",
				dialog_->contents());

    dialogCancelButton_ = new Wt::WPushButton("Cancel", dialog_->contents());
    dialogCancelButton_->clicked().connect(this, &PayPalApplication::cancel);

    dialog_->show();
  }

  void onPaymentApproval(const Wt::Payment::Approval& approval)
  {
    enableUpdates(false);
    triggerUpdate();

    if (!approval.error()) {
      switch (approval.outcome()) {
      case Wt::Payment::Approval::Accepted:
	updatePaymentDialog("<p>Payment successful.</p>"
			    "<p>Fetching payment details...</p>",
			    false);

        /* expressCheckout_->updateCustomerDetails() :
         * GetExpressCheckoutDetails API call: updates the customer
         * information.
         * .connect(this, &PayPalApplication::onCustomerDetails)
         * Call onCustomerDetails with paypal response(resulte).
         * The responce containes all the information that paypal has
         */
	expressCheckout_->updateCustomerDetails()
	  .connect(this, &PayPalApplication::onCustomerDetails);

	enableUpdates(true);

	break;

      case Wt::Payment::Approval::Denied:
      case Wt::Payment::Approval::Interrupted:
	cancel();

	break;
      }
    } else {
      cancel();
      showError("Error obtaining payment approval");
    }
  }

  void onCustomerDetails(const Wt::Payment::Result& result)
  {
    enableUpdates(false);
    triggerUpdate();

    clearDialog();

    if (!result.error()) {
      root()->clear();

      new Wt::WText(Wt::WString::tr("confirm.title"), root());

      printCustomer(expressCheckout_->customer());
      printOrder(expressCheckout_->order(), false);

      payment_ = new Wt::WContainerWidget(root());
      payment_->setStyleClass("payment");
      Wt::WPushButton *payB
	= new Wt::WPushButton("Confirm payment", payment_);

      payB->clicked().connect(this, &PayPalApplication::confirm);
      payB->clicked().connect(payB, &Wt::WPushButton::disable);
    } else {
      cancel();
      showError("Error while fetching payment details.");
    }

    printResultMessage(result);
  }

  void confirm()
  {
    expressCheckout_->
      completePayment(expressCheckout_->order().totalItemCost()).
      connect(this, &PayPalApplication::onConfirmed);
    enableUpdates(true);

    dialog_ = new Wt::WDialog("Information");
    dialogText_ = new Wt::WText("<p>Confirming payment...</p>",
				dialog_->contents());

    dialog_->show();
  }

  void onConfirmed(const Wt::Payment::Result& result)
  {
    enableUpdates(false);
    triggerUpdate();

    clearDialog();

    if (!result.error()) {
      root()->clear();
      new Wt::WText("Thanks for shopping with us !", root());
    } else {
      cancel();
      Wt::WString errorStr("Could not complete the payment. \n ");
      errorStr += result.errorMessage();
      showError(errorStr );
    }

    printResultMessage(result);
  }

  void updatePaymentDialog(const Wt::WString& text, bool enableCancel)
  {
    dialogText_->setText(text);
    dialogCancelButton_->setDisabled(!enableCancel);
  }

  void cancel()
  {
    clearDialog();

    if (expressCheckout_)
      delete expressCheckout_;

    expressCheckout_ = 0;

    reviewOrder();
  }

  void clearDialog()
  {
    delete dialog_;
    dialog_ = 0;
  }

  void showError(const Wt::WString& message)
  {
    dialog_ = new Wt::WMessageBox("Error", message, Wt::NoIcon, Wt::Ok);
    dialog_->exec();
  }

  void printResultMessage(const Wt::Payment::Result& result)
  {
    root()->addWidget(new Wt::WBreak());
    root()->addWidget(new Wt::WBreak());

    Wt::WPushButton *infoB = new Wt::WPushButton("Info", root());

    infoB->clicked().connect(
          boost::bind(&PayPalApplication::showInfo, this, result));
  }

  void printCustomer(const Wt::Payment::Customer& customer)
  {
    Wt::WTemplate *t = new Wt::WTemplate(Wt::WString::tr("customer.info"),
					 root());

    t->bindString("firstName", customer.firstName());
    t->bindString("lastName", customer.lastName());
    t->bindString("email", customer.email());
    t->bindString("locale", customer.locale());
    t->bindString("payerId", customer.payerId());
    t->bindString("street1", customer.shippingAddress().street1());
    t->bindString("street2", customer.shippingAddress().street2());
    t->bindString("zip", customer.shippingAddress().zip());
    t->bindString("city", customer.shippingAddress().city());
    t->bindString("state", customer.shippingAddress().state());
    t->bindString("countryCode", customer.shippingAddress().countryCode());
    t->bindString("phoneNumber", customer.shippingAddress().phoneNumber());
  }

  void printOrder(const Wt::Payment::Order &order, bool showDetails)
  {
    Wt::WTemplate *t = new Wt::WTemplate(Wt::WString::tr("order.info"),
					 root());

    t->bindString("itemTotalCost", order.totalItemCost().toString());
    t->bindString("tax", order.tax().toString());
    t->bindString("handlingCost", order.handling().toString());
    t->bindString("shippingCost", order.shipping().toString());
    t->bindString("shippingDiscount", order.shippingDiscount().toString());
    t->bindString("shippingInsurance", order.shippingInsurance().toString());
    t->bindString("totalCost", order.totalOrderCost().toString());

    if (showDetails && order.items().size() > 0) {
      Wt::WContainerWidget *items = new Wt::WContainerWidget();
      items->addStyleClass("items");
      items->addWidget(new Wt::WText(Wt::WString::tr("items.header")));

      for (unsigned i = 0; i < order.items().size(); ++i) {
	const Wt::Payment::OrderItem &item = order.items()[i];
	Wt::WTemplate *it = new Wt::WTemplate(Wt::WString::tr("item.info"));
	it->bindString("name", item.name());
	it->bindString("description", item.description());
	it->bindString("number", item.number());
	it->bindString("quantity",
		       boost::lexical_cast<std::string>(item.quantity()));
	it->bindString("unitCost", item.unitCost().toString());
	it->bindString("totalCost",
		       (item.unitCost() * item.quantity()).toString());

	items->addWidget(it);
      }
      
      t->bindWidget("items", items);
    } else
      t->bindEmpty("items");
  }

  void showInfo(const Wt::Payment::Result& result)
  {
    Wt::WDialog dialog("Info");
    Wt::WContainerWidget *container = new Wt::WContainerWidget();

    container->addWidget(new Wt::WText("Request message:"));
    container->addWidget(new Wt::WBreak());
    container->addWidget(new Wt::WBreak());

    std::map<std::string, std::string> map = result.requestMessage();
    std::map<std::string, std::string>::iterator it;

    for (it = map.begin(); it != map.end(); ++it) {
      Wt::WStringStream text;
      text << (*it).first << " = "
           << Wt::Utils::urlDecode((*it).second);
      container->addWidget(new Wt::WText(Wt::WString(text.str())));
      container->addWidget(new Wt::WBreak());
    }

    container->addWidget(new Wt::WBreak());
    container->addWidget(new Wt::WText("Paypal response message:"));
    container->addWidget(new Wt::WBreak());
    container->addWidget(new Wt::WBreak());

    map = result.responseMessage();

    for(it = map.begin(); it != map.end(); ++it){
      Wt::WStringStream text;
      text << (*it).first << " = " << (*it).second;
      container->addWidget(new Wt::WText(Wt::WString(text.str())));
      container->addWidget(new Wt::WBreak());
    }

    container->setOverflow(Wt::WContainerWidget::OverflowScroll);
    dialog.resize("70%", "70%");

    Wt::WVBoxLayout *layout = new Wt::WVBoxLayout(dialog.contents());
    layout->addWidget(container, 1);

    Wt::WPushButton ok("Close");
    layout->addWidget(&ok, 0, Wt::AlignCenter);

    ok.clicked().connect(&dialog, &Wt::WDialog::accept);

    dialog.exec();
  }

  Wt::Payment::Order createOrder()
  {
    Wt::Payment::OrderItem item1, item2;

    item1.setName("Waffle Maker");
    item1.setNumber("00001");
    item1.setDescription("Emweb FlipSide Belgian Waffle Maker");
    item1.setQuantity(1);
    item1.setUnitCost(Wt::Payment::Money(49, 99, "USD"));

    item2.setName("Waffle Mix");
    item2.setNumber("00002");
    item2.setDescription("Mix for authentic Belgian Luikse Waffles");
    item2.setQuantity(2);
    item2.setUnitCost(Wt::Payment::Money(4, 99, "USD"));

    Wt::Payment::Order order;

    order.items().push_back(item1);
    order.items().push_back(item2);

    order.setHandling(Wt::Payment::Money(0, 99, "USD"));
    order.setShipping(Wt::Payment::Money(7, 1, "USD"));
    order.setShippingDiscount(Wt::Payment::Money(-7, 0, "USD"));
    order.setShippingInsurance(Wt::Payment::Money(2, 23, "USD"));
    order.setTax(Wt::Payment::Money(11, 63, "USD"));

    order.setTotalItemCost(order.computeTotalItemCost());
    order.setTotalOrderCost(order.computeTotalOrderCost());

    return order;
  }

  Wt::Payment::Customer createCustomer()
  {
    Wt::Payment::Customer customer;

    /*
     * This is optional, may help a user register with Paypal.
     */

    /*
    customer.setEmail("Waffle@emwab.be");
    customer.setFirstName("Name");
    customer.setLastName("LastName");

    Wt::Payment::Address address;
    address.setCity("Leuven");
    address.setCountryCode("BE");
    address.setPhoneNumber("123456789");
    address.setStreet2("Brusselsstraat");

    customer.setShippingAddress(address);
    */

    return customer;
  }

};

Wt::WApplication *createApplication(const Wt:: WEnvironment& env)
{
  return new PayPalApplication(env);
}

int main(int argc, char **argv)
{
  try {
    Wt::WServer server(argc, argv, WTHTTP_CONFIGURATION);

    server.addEntryPoint(Wt::Application, createApplication);

    payPalService = new Wt::Payment::PayPalService();

    if (!payPalService->configureFromProperties())
      payPalService->configureTestSandbox();

    server.run();
  } catch (Wt::WServer::Exception& e) {
    std::cerr << e.what() << std::endl;
  } catch (std::exception &e) {
    std::cerr << "exception: " << e.what() << std::endl;
  }

  delete payPalService;
}
