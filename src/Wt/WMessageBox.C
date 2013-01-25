/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "Wt/WContainerWidget"
#include "Wt/WImage"
#include "Wt/WMessageBox"
#include "Wt/WPushButton"
#include "Wt/WSignalMapper"
#include "Wt/WText"

namespace Wt {

StandardButton WMessageBox::order_[] = { Ok, Yes, YesAll, Retry, No,
					 NoAll, Abort, Ignore, Cancel };

const char *WMessageBox::buttonText_[]
  = { "Wt.WMessageBox.Ok", "Wt.WMessageBox.Yes", "Wt.WMessageBox.YesToAll",
      "Wt.WMessageBox.Retry", "Wt.WMessageBox.No", "Wt.WMessageBox.NoToAll",
      "Wt.WMessageBox.Abort", "Wt.WMessageBox.Ignore", "Wt.WMessageBox.Cancel"
    };

const char *WMessageBox::iconURI[]
  = { "icons/information.png",
      "icons/warning.png",
      "icons/critical.png",
      "icons/question.png" };

WMessageBox::WMessageBox(WObject *parent)
  : WDialog(parent),
    buttons_(0),
    icon_(NoIcon),
    result_(NoButton),
    buttonClicked_(this)
{
  create();
}

WMessageBox::WMessageBox(const WString& caption, const WString& text,
			 Icon icon, WFlags<StandardButton> buttons,
			 WObject *parent)
  : WDialog(caption, parent),
    buttons_(0),
    icon_(NoIcon),
    buttonClicked_(this)
{
  create();

  setText(text);
  setIcon(icon);
  setButtons(buttons);
}

WPushButton *WMessageBox::addButton(const WString& text, StandardButton result)
{
  WPushButton *b = new WPushButton(text, footer());
  buttonMapper_->mapConnect(b->clicked(), result);

  return b;
}

void WMessageBox::create()
{
  iconImage_ = 0;
  text_ = new WText(contents());
  WContainerWidget *buttons = new WContainerWidget(contents());
  buttons->setMargin(WLength(3), Top);
  buttons->setPadding(WLength(5), Left|Right);
  buttonMapper_ = new WSignalMapper<StandardButton>(this);
  buttonMapper_->mapped().connect(this, &WMessageBox::onButtonClick);

  //buttonMapper_->mapConnect(contents()->escapePressed, Cancel);
  //contents()->escapePressed.preventDefault();

  rejectWhenEscapePressed();
}

void WMessageBox::setText(const WString& text)
{
  text_->setText(text);
}

const WString& WMessageBox::text() const
{
  return text_->text();
}

void WMessageBox::setIcon(Icon icon)
{
  icon_ = icon;

  /*
   * Ignore icons for now... Icons are so desktop ?
   */
  if (false && icon_ != NoIcon) {
    if (!iconImage_) {
      iconImage_ = new WImage(WLink(iconURI[icon_ - 1]));
      contents()->insertBefore(iconImage_, text_);
    } else
      iconImage_->setImageLink(WLink(iconURI[icon_ - 1]));
  } else {
    delete iconImage_;
    iconImage_ = 0;
  }
}

void WMessageBox::setButtons(WFlags<StandardButton> buttons)
{
  buttons_ = buttons;
  footer()->clear();

  for (int i = 0; i < 9; ++i)
    if (buttons_ & order_[i]) {
      WPushButton *b
	= new WPushButton(tr(buttonText_[i]), footer());
      buttonMapper_->mapConnect(b->clicked(), order_[i]);

      if (order_[i] == Ok || order_[i] == Yes)
	b->setFocus();
    }
}

WPushButton *WMessageBox::button(StandardButton b)
{
  int index = 0;
  for (int i = 0; i <= 9; ++i)
    if (buttons_ & order_[i]) {
      if (order_[i] == b)
	return
	  dynamic_cast<WPushButton *>(footer()->children()[index]);
      ++index;
    }

  return 0;
}

void WMessageBox::onButtonClick(StandardButton b)
{
  result_ = b;
  buttonClicked_.emit(b);
}

StandardButton WMessageBox::show(const WString& caption,
				 const WString& text,
				 WFlags<StandardButton> buttons,
				 const WAnimation& animation)
{
  WMessageBox box(caption, text, Information, buttons);
  box.buttonClicked().connect(&box, &WMessageBox::accept);

  box.exec(animation);

  return box.buttonResult();
}

}
