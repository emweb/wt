#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WCheckBox.h>
#include <Wt/WCssDecorationStyle.h>
#include <Wt/WFont.h>
#include <Wt/WLength.h>
#include <Wt/WLineEdit.h>
#include <Wt/WPushButton.h>
#include <Wt/WServer.h>
#include <Wt/WSpinBox.h>
#include <Wt/WStringStream.h>
#include <Wt/WText.h>

#include <Wt/Http/Client.h>
#include <Wt/Http/Message.h>

class HttpClientApplication : public Wt::WApplication {
public:
  HttpClientApplication(const Wt::WEnvironment &env);

private:
  void doRequest();
  void requestDone(Wt::AsioWrapper::error_code ec, const Wt::Http::Message &msg);

  Wt::WContainerWidget *requestForm_;
  Wt::WLineEdit *url_;
  Wt::WText *result_;
  Wt::WCheckBox *followRedirect_;
  Wt::WSpinBox *maxRedirects_;
  Wt::Http::Client client_;
};

HttpClientApplication::HttpClientApplication(const Wt::WEnvironment &env)
  : WApplication(env)
{
  root()->decorationStyle().setFont(Wt::WFont(Wt::FontFamily::SansSerif));

  requestForm_ = root()->addNew<Wt::WContainerWidget>();

  url_ = requestForm_->addNew<Wt::WLineEdit>();
  url_->setPlaceholderText("enter a URL...");
  url_->setWidth(Wt::WLength(300, Wt::LengthUnit::Pixel));

  Wt::WPushButton *goBtn = requestForm_->addNew<Wt::WPushButton>(Wt::utf8("Go"));
  goBtn->clicked().connect(this, &HttpClientApplication::doRequest);

  requestForm_->addNew<Wt::WBreak>();

  followRedirect_ = requestForm_->addNew<Wt::WCheckBox>(Wt::utf8("Follow redirect"));
  followRedirect_->setChecked(false);

  requestForm_->addNew<Wt::WBreak>();

  requestForm_->addNew<Wt::WText>("Max redirects: ");

  maxRedirects_ = requestForm_->addNew<Wt::WSpinBox>();
  maxRedirects_->setRange(1, 20);
  maxRedirects_->setValue(10);
  maxRedirects_->setWidth(Wt::WLength(40, Wt::LengthUnit::Pixel));

  result_ = root()->addNew<Wt::WText>();
  result_->decorationStyle().setFont(Wt::WFont(Wt::FontFamily::Monospace));
  result_->setTextFormat(Wt::TextFormat::Plain);

  client_.setMaxRedirects(10);
  client_.done().connect(this, &HttpClientApplication::requestDone);
}

void HttpClientApplication::doRequest()
{
  client_.setFollowRedirect(followRedirect_->isChecked());
  client_.setMaxRedirects(maxRedirects_->value());
  if (client_.get(url_->text().toUTF8())) {
    enableUpdates(true);
    result_->setText(Wt::utf8("Waiting for response..."));
    requestForm_->setDisabled(true);
  }
}

void HttpClientApplication::requestDone(Wt::AsioWrapper::error_code ec, const Wt::Http::Message &msg)
{
  if (!ec) {
    Wt::WStringStream ss;
    ss << "Status code " << msg.status() << "\n\n"
       << msg.body();
    result_->setText(Wt::utf8(ss.str()));
  } else {
    result_->setText(Wt::utf8("Error: {1}").arg(ec.message()));
  }
  requestForm_->setDisabled(false);
  triggerUpdate();
  enableUpdates(false);
}

std::unique_ptr<Wt::WApplication> createApplication(const Wt::WEnvironment &env)
{
  return std::make_unique<HttpClientApplication>(env);
}

int main(int argc, char *argv[])
{
  return Wt::WRun(argc, argv, &createApplication);
}
