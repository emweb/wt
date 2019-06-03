#include <Wt/WApplication>
#include <Wt/WContainerWidget>
#include <Wt/WCheckBox>
#include <Wt/WCssDecorationStyle>
#include <Wt/WFont>
#include <Wt/WLength>
#include <Wt/WLineEdit>
#include <Wt/WPushButton>
#include <Wt/WServer>
#include <Wt/WSpinBox>
#include <Wt/WStringStream>
#include <Wt/WText>

#include <Wt/Http/Client>
#include <Wt/Http/Message>

class HttpClientApplication : public Wt::WApplication {
public:
  HttpClientApplication(const Wt::WEnvironment &env);

private:
  void doRequest();
  void requestDone(boost::system::error_code ec, const Wt::Http::Message &msg);

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
  root()->decorationStyle().setFont(Wt::WFont(Wt::WFont::SansSerif));

  requestForm_ = new Wt::WContainerWidget(root());

  url_ = new Wt::WLineEdit(requestForm_);
  url_->setPlaceholderText("enter a URL...");
  url_->setWidth(Wt::WLength(300, Wt::WLength::Pixel));

  Wt::WPushButton *goBtn = new Wt::WPushButton(Wt::utf8("Go"), requestForm_);
  goBtn->clicked().connect(this, &HttpClientApplication::doRequest);

  new Wt::WBreak(requestForm_);

  followRedirect_ = new Wt::WCheckBox(Wt::utf8("Follow redirect"), requestForm_);
  followRedirect_->setChecked(false);

  new Wt::WBreak(requestForm_);

  new Wt::WText("Max redirects: ", requestForm_);

  maxRedirects_ = new Wt::WSpinBox(requestForm_);
  maxRedirects_->setRange(1, 20);
  maxRedirects_->setValue(10);
  maxRedirects_->setWidth(Wt::WLength(40, Wt::WLength::Pixel));

  result_ = new Wt::WText(root());
  result_->decorationStyle().setFont(Wt::WFont(Wt::WFont::Monospace));
  result_->setTextFormat(Wt::PlainText);

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

void HttpClientApplication::requestDone(boost::system::error_code ec, const Wt::Http::Message &msg)
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

Wt::WApplication *createApplication(const Wt::WEnvironment &env)
{
  return new HttpClientApplication(env);
}

int main(int argc, char *argv[])
{
  return Wt::WRun(argc, argv, &createApplication);
}
