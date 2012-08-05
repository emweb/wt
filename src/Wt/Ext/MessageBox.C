/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "Wt/Ext/MessageBox"

#include "Wt/WApplication"
#include "Wt/WEnvironment"
#include "Wt/WException"

namespace Wt {
  namespace Ext {

const char *MessageBox::buttonText_[] = { "Ok", "Yes", "No", "Cancel" };

MessageBox::MessageBox(bool i18n)
  : Dialog(Bla()),
    buttonClicked_(this),
    buttons_(NoButton),
    i18n_(i18n),
    prompt_(false),
    textArea_(false),
    progress_(false),
    progressValue_(0),
    progressInfinite_(false),
    firstDisplay_(true),
    catchDelete_(0),
    result_(NoButton),
    extButtonClicked_(this, "btnclk", false)
{
  extButtonClicked_.connect(this, &MessageBox::onClick);
}

MessageBox::MessageBox(const WString& caption, const WString& text,
		       Icon icon, WFlags<StandardButton> buttons, bool i18n)
  : Dialog(Bla()),
    text_(text),
    buttons_(buttons),
    icon_(icon),
    i18n_(i18n),
    prompt_(false),
    textArea_(false),
    progress_(false),
    progressValue_(0),
    progressInfinite_(false),
    firstDisplay_(true),
    catchDelete_(0),
    result_(NoButton),
    extButtonClicked_(this, "btnclk", false)
{
  setWindowTitle(caption);
  extButtonClicked_.connect(this, &MessageBox::onClick);
}

MessageBox::~MessageBox()
{
  hide();

  if (catchDelete_)
    *catchDelete_ = true;
}

void MessageBox::setText(const WString& text)
{
  text_ = text;
  if (!hidden_)
    WApplication::instance()
      ->doJavaScript(elRef() + ".updateText(" + text.jsStringLiteral() + ");");
}

void MessageBox::setIcon(Icon icon)
{
  icon_ = icon;
}

void MessageBox::setButtons(WFlags<StandardButton> buttons)
{
  buttons_ = buttons;
}

void MessageBox::enablePrompt(bool enable)
{
  prompt_ = enable;
}

void MessageBox::enableTextArea(bool enable)
{
  textArea_ = enable;
}

void MessageBox::enableProgressBar(bool enable, bool infinite)
{
  progress_ = enable;
  progressInfinite_ = infinite;
}

void MessageBox::updateProgress(double v)
{
  progressValue_ = std::min(1.0, std::max(0.0, v));

  if (!hidden_)
    WApplication::instance()
      ->doJavaScript(elRef() + ".updateProgress("
		     + boost::lexical_cast<std::string>(progressValue_) + ");");
}

void MessageBox::setValue(const WString& value)
{
  value_ = value;
}

void MessageBox::setHidden(bool hidden, const WAnimation& animation)
{
  if (hidden != hidden_) {
    hidden_ = hidden;

    WApplication *app = WApplication::instance();

    if (!hidden)
      setExposeMask(app);
    else
      restoreExposeMask(app);
 
    if (hidden)
      app->doJavaScript(elRef() + ".hide();");
    else {
      std::stringstream config;
      config << "{a:0";
      createConfig(config);
      config << "}";

      std::string var;
      if (firstDisplay_) {
	var = elRef() + "=Ext.Msg";

	/* fix cursor problem in FF 1.5, 2 */
	if (!app->environment().agentIsIE())
	  app->doJavaScript
	  ("Ext.Msg.getDialog().on('show', function(d) {"
	   "var div = Ext.get(d.el);"
	   "div.setStyle('overflow', 'auto');"
	   "var text = div.select('.ext-mb-textarea', true);"
	   "if (!text.item(0))"
	   "text = div.select('.ext-mb-text', true);"
	   "if (text.item(0))"
	   "text.item(0).dom.select();});");
      } else
	var = elRef();

      WApplication::instance()
	->doJavaScript(var + ".show(" + config.str() + ");");

      if (progress_) {
	WApplication::instance()
	  ->doJavaScript(elRef() + ".updateProgress("
			 + boost::lexical_cast<std::string>(progressValue_)
			 + ");");
      }

      firstDisplay_ = false;
    }
  }
}

void MessageBox::refresh()
{
  if (text_.refresh())
    setText(text_);

  Dialog::refresh();
}

void MessageBox::createConfig(std::ostream& config)
{
  config << ",title:" << windowTitle().jsStringLiteral();

  if (!text_.empty())
    config << ",msg:" << text_.jsStringLiteral();
  if (prompt_)
    config << ",prompt:true";
  if (textArea_)
    config << ",multiline:true";
  if (prompt_ || textArea_)
    config << ",value:" << value_.jsStringLiteral();
  if (progress_)
    config << ",progress:true";
  if (buttons_ != NoButton) {
    config << ",buttons:{a:0";
    if (buttons_ & Ok)
      config << ",ok:" + jsStringLiteral(buttonText(0), '\'');
    if (buttons_ & Cancel)
      config << ",cancel:" + jsStringLiteral(buttonText(3), '\'');
    if (buttons_ & Yes)
      config << ",yes:" + jsStringLiteral(buttonText(1), '\'');
    if (buttons_ & No)
      config << ",no:" + jsStringLiteral(buttonText(2), '\'');
    config << "}";
  }

  config << ",fn:function(b,v){"
	 << extButtonClicked_.createCall("b","v") << "}";
}

std::string MessageBox::renderRemoveJs()
{
  throw WException("MessageBox::renderRemoveJs(): really?");
}

std::string MessageBox::buttonText(int buttonIndex) const
{
  if (i18n_)
    return WString::tr(buttonText_[buttonIndex]).toUTF8();
  else
    return buttonText_[buttonIndex];
}

StandardButton MessageBox::show(const WString& caption,
				const WString& text,
				WFlags<StandardButton> buttons, bool i18n)
{
  MessageBox box(caption, text, NoIcon, buttons, i18n);

  box.exec();

  return box.result();
}

StandardButton MessageBox::prompt(const WString& caption,
				  const WString& text,
				  WString& result,
				  bool multiLine, bool i18n)
{
  MessageBox box(caption, text, NoIcon, Ok | Cancel, i18n);
  if (multiLine)
    box.enableTextArea(true);
  else
    box.enablePrompt(true);
  box.setValue(result);
  box.exec();

  if (box.result() == Ok)
    result = box.value();

  return box.result();
}

void MessageBox::onClick(std::string buttonId, std::string value)
{
  hidden_ = true;
  restoreExposeMask(WApplication::instance());

  StandardButton b = NoButton;

  if      (buttonId == "ok")     b = Ok;
  else if (buttonId == "cancel") b = Cancel;
  else if (buttonId == "yes")    b = Yes;
  else if (buttonId == "no")     b = No;
  else
    throw WException("MessageBox: internal error, unknown buttonId '"
		     + buttonId + "';");

  bool accepted = b == Ok || b == Yes;

  if (accepted)
    value_ = WString::fromUTF8(value);
  result_ = b;
    
  bool wasDeleted = false;
  catchDelete_ = &wasDeleted;

  done(accepted ? Accepted : Rejected);

  if (!wasDeleted) {
    buttonClicked_.emit(b);
    catchDelete_ = 0;
  }
}

  }
}
