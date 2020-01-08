/*
 * Copyright (C) 2012 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <Wt/WAny.h>
#include <Wt/WAnchor.h>
#include <Wt/WApplication.h>
#include <Wt/WBreak.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WDialog.h>
#include <Wt/WGridLayout.h>
#include <Wt/WImage.h>
#include <Wt/WLabel.h>
#include <Wt/WMessageBox.h>
#include <Wt/WPushButton.h>
#include <Wt/WServer.h>
#include <Wt/WStringStream.h>
#include <Wt/WTable.h>
#include <Wt/WTemplate.h>
#include <Wt/WText.h>
#include <Wt/WVBoxLayout.h>
#include <Wt/Utils.h>

#include <Wt/Payment/PayPal.h>
#include <Wt/Payment/Customer.h>
#include <Wt/Payment/Money.h>
#include <Wt/Payment/Order.h>
#include <Wt/Payment/Address.h>

using namespace Wt;

std::unique_ptr<Payment::PayPalService> payPalService;
const char *PAYPAL_BUTTON_URL =
  "https://www.paypal.com/en_US/i/btn/btn_xpressCheckout.gif";

class PayPalApplication : public WApplication
{
public:
  PayPalApplication(const WEnvironment& env)
    : WApplication(env),
      payment_(0)
  {
    messageResourceBundle().use("text");
    useStyleSheet("css/style.css");

    std::unique_ptr<WPushButton> checkout
        = cpp14::make_unique<WPushButton>("Proceed to checkout...");
    checkout->clicked().connect(this, &PayPalApplication::reviewOrder);

    WTemplate *t = root()->addWidget(cpp14::make_unique<WTemplate>(WString::tr("start")));
    t->bindWidget("checkout", std::move(checkout));
  }

  ~PayPalApplication()
  {
    clearDialog();
  }

private:
  std::unique_ptr<Payment::PayPalExpressCheckout> expressCheckout_ = nullptr;
  WContainerWidget *payment_;

  std::unique_ptr<WDialog> dialog_ = nullptr;
  WText *dialogText_;
  WPushButton *dialogCancelButton_;

  void reviewOrder()
  {
    /*
     * First page, shows order and pay pal button
     */

    Payment::Order order = createOrder();
    Payment::Customer customer = createCustomer();

    root()->clear();
    root()->addWidget(cpp14::make_unique<WText>(WString::tr("review.title")));
    printOrder(order, true);

    expressCheckout_.reset(payPalService->createExpressCheckout(customer, order));

    std::string logo = "http://www.webtoolkit.eu/css/wt/emweb_powered.jpg";
    expressCheckout_->setParameter("LOGOIMG", logo);
    expressCheckout_->setParameter("CARTBORDERCOLOR", "000CD");

    expressCheckout_->setup().connect(this, &PayPalApplication::onSetup);
    enableUpdates(true);

    payment_ = root()->addWidget(cpp14::make_unique<WContainerWidget>());
    payment_->setStyleClass("payment");
    payment_->addWidget(cpp14::make_unique<WText>("Preparing payment..."));
  }

  void onSetup(const Payment::Result& result)
  {
    enableUpdates(false);
    triggerUpdate();

    if (!result.error()) {
      payment_->clear();

      // Create the PayPal button
      WAnchor *payButton = payment_->addWidget(cpp14::make_unique<WAnchor>());
      payButton->setImage(cpp14::make_unique<WImage>(PAYPAL_BUTTON_URL));

      // Clicking starts the payment process
      payButton->clicked().connect
        (expressCheckout_.get(),
         &Payment::PayPalExpressCheckout::startPayment);

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

    dialog_ = cpp14::make_unique<WDialog>("Information");

    dialogText_ = dialog_->contents()->addWidget(cpp14::make_unique<WText>("<p>Payment using PayPal in progress ...</p>"));

    dialogCancelButton_ = dialog_->contents()->addWidget(cpp14::make_unique<WPushButton>("Cancel"));
    dialogCancelButton_->clicked().connect(this, &PayPalApplication::cancel);

    dialog_->show();
  }

  void onPaymentApproval(const Payment::Approval& approval)
  {
    enableUpdates(false);
    triggerUpdate();

    if (!approval.error()) {
      switch (approval.outcome()) {
      case Payment::ApprovalOutcome::Accepted:
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

      case Payment::ApprovalOutcome::Denied:
      case Payment::ApprovalOutcome::Interrupted:
	cancel();

        break;
      }
    } else {
      cancel();
      showError("Error obtaining payment approval");
    }
  }

  void onCustomerDetails(const Payment::Result& result)
  {
    enableUpdates(false);
    triggerUpdate();

    clearDialog();

    if (!result.error()) {
      root()->clear();

      root()->addWidget(cpp14::make_unique<WText>(WString::tr("confirm.title")));

      printCustomer(expressCheckout_->customer());
      printOrder(expressCheckout_->order(), false);

      payment_ = root()->addWidget(cpp14::make_unique<WContainerWidget>());
      payment_->setStyleClass("payment");

      auto *payB
        = payment_->addWidget(cpp14::make_unique<WPushButton>("Confirm payment"));

      payB->clicked().connect(this, &PayPalApplication::confirm);
      payB->clicked().connect(payB, &WPushButton::disable);
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

    dialog_ = cpp14::make_unique<WDialog>("Information");
    dialogText_ = dialog_->contents()->addWidget(cpp14::make_unique<WText>(
                                                   "<p>Confirming payment...</p>"));
    dialog_->show();
  }

  void onConfirmed(const Payment::Result& result)
  {
    enableUpdates(false);
    triggerUpdate();

    clearDialog();

    if (!result.error()) {
      root()->clear();
      root()->addWidget(cpp14::make_unique<WText>("Thanks for shopping with us !"));
    } else {
      cancel();
      WString errorStr("Could not complete the payment. \n ");
      errorStr += result.errorMessage();
      showError(errorStr );
    }

    printResultMessage(result);
  }

  void updatePaymentDialog(const WString& text, bool enableCancel)
  {
    dialogText_->setText(text);
    dialogCancelButton_->setDisabled(!enableCancel);
  }

  void cancel()
  {
    clearDialog();
    expressCheckout_.reset();
    reviewOrder();
  }

  void clearDialog()
  {
    dialog_.reset();
  }

  void showError(const WString& message)
  {
    WMessageBox::show("Error", message, StandardButton::Ok);
  }

  void printResultMessage(const Payment::Result& result)
  {
    root()->addWidget(cpp14::make_unique<WBreak>());
    root()->addWidget(cpp14::make_unique<WBreak>());

    auto *infoB = root()->addWidget(cpp14::make_unique<WPushButton>("Info"));

    infoB->clicked().connect(
          std::bind(&PayPalApplication::showInfo, this, result));
  }

  void printCustomer(const Payment::Customer& customer)
  {
    WTemplate *t = root()->addWidget(cpp14::make_unique<WTemplate>(WString::tr("customer.info")));

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

  void printOrder(const Payment::Order &order, bool showDetails)
  {
    WTemplate *t = root()->addWidget(cpp14::make_unique<WTemplate>(WString::tr("order.info")));

    t->bindString("itemTotalCost", order.totalItemCost().toString());
    t->bindString("tax", order.tax().toString());
    t->bindString("handlingCost", order.handling().toString());
    t->bindString("shippingCost", order.shipping().toString());
    t->bindString("shippingDiscount", order.shippingDiscount().toString());
    t->bindString("shippingInsurance", order.shippingInsurance().toString());
    t->bindString("totalCost", order.totalOrderCost().toString());

    if (showDetails && order.items().size() > 0) {
      std::unique_ptr<WContainerWidget> items
          = cpp14::make_unique<WContainerWidget>();
      items->addStyleClass("items");
      items->addWidget(cpp14::make_unique<WText>(WString::tr("items.header")));

      for (auto item : order.items()) {
        std::unique_ptr<WTemplate> it
            = cpp14::make_unique<WTemplate>(WString::tr("item.info"));
	it->bindString("name", item.name());
	it->bindString("description", item.description());
	it->bindString("number", item.number());
	it->bindString("quantity",
		       asString(item.quantity()));
	it->bindString("unitCost", item.unitCost().toString());
	it->bindString("totalCost",
		       (item.unitCost() * item.quantity()).toString());

        items->addWidget(std::move(it));
      }
      
      t->bindWidget("items", std::move(items));
    } else
      t->bindEmpty("items");
  }

  void showInfo(const Payment::Result& result)
  {
    WDialog dialog("Info");
    std::unique_ptr<WContainerWidget> container
        = cpp14::make_unique<WContainerWidget>();

    container->addWidget(cpp14::make_unique<WText>("Request message:"));
    container->addWidget(cpp14::make_unique<WBreak>());
    container->addWidget(cpp14::make_unique<WBreak>());

    std::map<std::string, std::string> map = result.requestMessage();

    for (auto& i : map) {
      WStringStream text;
      text << i.first << " = "
           << Utils::urlDecode(i.second);
      container->addWidget(cpp14::make_unique<WText>(WString(text.str())));
      container->addWidget(cpp14::make_unique<WBreak>());
    }

    container->addWidget(cpp14::make_unique<WBreak>());
    container->addWidget(cpp14::make_unique<WText>("Paypal response message:"));
    container->addWidget(cpp14::make_unique<WBreak>());
    container->addWidget(cpp14::make_unique<WBreak>());

    map = result.responseMessage();

    for(auto& i : map) {
      WStringStream text;
      text << i.first << " = " << i.second;
      container->addWidget(cpp14::make_unique<WText>(WString(text.str())));
      container->addWidget(cpp14::make_unique<WBreak>());
    }

    container->setOverflow(Overflow::Scroll);
    dialog.resize("70%", "70%");

    auto layout = dialog.contents()->setLayout(cpp14::make_unique<WVBoxLayout>());
    layout->addWidget(std::move(container), 1);

    auto ok = layout->addWidget(cpp14::make_unique<WPushButton>("Close"),
                                0, AlignmentFlag::Center);
    ok->clicked().connect(&dialog, &WDialog::accept);

    dialog.exec();
  }

  Payment::Order createOrder()
  {
    Payment::OrderItem item1, item2;

    item1.setName("Waffle Maker");
    item1.setNumber("00001");
    item1.setDescription("Emweb FlipSide Belgian Waffle Maker");
    item1.setQuantity(1);
    item1.setUnitCost(Payment::Money(49, 99, "USD"));

    item2.setName("Waffle Mix");
    item2.setNumber("00002");
    item2.setDescription("Mix for authentic Belgian Luikse Waffles");
    item2.setQuantity(2);
    item2.setUnitCost(Payment::Money(4, 99, "USD"));

    Payment::Order order;

    order.items().push_back(item1);
    order.items().push_back(item2);

    order.setHandling(Payment::Money(0, 99, "USD"));
    order.setShipping(Payment::Money(7, 1, "USD"));
    order.setShippingDiscount(Payment::Money(-7, 0, "USD"));
    order.setShippingInsurance(Payment::Money(2, 23, "USD"));
    order.setTax(Payment::Money(11, 63, "USD"));

    order.setTotalItemCost(order.computeTotalItemCost());
    order.setTotalOrderCost(order.computeTotalOrderCost());

    return order;
  }

  Payment::Customer createCustomer()
  {
    Payment::Customer customer;

    /*
     * This is optional, may help a user register with Paypal.
     */

    /*
    customer.setEmail("Waffle@emwab.be");
    customer.setFirstName("Name");
    customer.setLastName("LastName");

    Payment::Address address;
    address.setCity("Leuven");
    address.setCountryCode("BE");
    address.setPhoneNumber("123456789");
    address.setStreet2("Brusselsstraat");

    customer.setShippingAddress(address);
    */

    return customer;
  }

};

std::unique_ptr<WApplication> createApplication(const WEnvironment& env)
{
  return cpp14::make_unique<PayPalApplication>(env);
}

int main(int argc, char **argv)
{
  try {
    WServer server(argc, argv, WTHTTP_CONFIGURATION);

    server.addEntryPoint(EntryPointType::Application, createApplication);

    payPalService = cpp14::make_unique<Payment::PayPalService>();

    if (!payPalService->configureFromProperties())
      payPalService->configureTestSandbox();

    server.run();
  } catch (WServer::Exception& e) {
    std::cerr << e.what() << std::endl;
  } catch (std::exception &e) {
    std::cerr << "exception: " << e.what() << std::endl;
  }

  payPalService.reset();
}
