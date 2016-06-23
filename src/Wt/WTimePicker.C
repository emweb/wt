#include "WTimePicker"

#include "WStringStream"
#include "WTemplate"
#include "WPushButton"
#include "WText"
#include "WIcon"
#include "WContainerWidget"
#include "WSpinBox"
#include "WTimeValidator"
#include "WTimeEdit"

namespace Wt {

LOGGER("WTimePicker");

WTimePicker::WTimePicker(WContainerWidget *parent)
    : WCompositeWidget(parent),
      selectionChanged_(this)
{
    init();
}

WTimePicker::WTimePicker(const WTime &time, WContainerWidget *parent)
    : WCompositeWidget(parent)
{
    init(time);
}

WTimePicker::WTimePicker(WTimeEdit *timeEdit, WContainerWidget *parent)
    : WCompositeWidget(parent), timeEdit_(timeEdit)
{
    init();
}

WTimePicker::WTimePicker(const WTime &time, WTimeEdit *timeEdit, WContainerWidget *parent)
    : WCompositeWidget(parent), timeEdit_(timeEdit)
{
    init(time);
}

WTimePicker::~WTimePicker()
{
  // The widgets may or may not be bound to the template. Take them and delete them
  // to make sure they're all properly deleted.
  WTemplate *container = dynamic_cast<WTemplate *>(implementation());
  container->takeWidget("hour");
  container->takeWidget("minute");
  container->takeWidget("second");
  container->takeWidget("millisecond");
  container->takeWidget("ampm");

  delete sbhour_;
  delete sbminute_;
  delete sbsecond_;
  delete sbmillisecond_;
  delete cbAP_;
}

void WTimePicker::init(const WTime &time)
{
    WTemplate *container = new WTemplate();
    setImplementation(container);
    container->addStyleClass("form-inline");
    container->setTemplateText(tr("Wt.WTimePicker.template"));
    createWidgets();
    configure();
}

WTime WTimePicker::time() const
{
    int hours = 0, minutes = 0, seconds = 0, milliseconds = 0;

    try {
        hours = boost::lexical_cast<int>(sbhour_->text().toUTF8());
        minutes = boost::lexical_cast<int>(sbminute_->text().toUTF8());
        seconds = boost::lexical_cast<int>(sbsecond_->text().toUTF8());

        if (formatMs())
            milliseconds = boost::lexical_cast<int>(sbmillisecond_->text().toUTF8());
	if (formatAp()) {
	  if (cbAP_->currentIndex() == 1) {
	    if (hours != 12)
	      hours += 12;
	  } else
	    if (hours == 12)
	      hours = 0;
	}
    } catch(const boost::bad_lexical_cast& ex) {
        LOG_ERROR("boost::bad_lexical_cast caught in WTimePicker::time()");
    }

    return WTime(hours, minutes, seconds, milliseconds);
}

void WTimePicker::setTime(const WTime& time)
{
    if (!time.isValid()) {
        LOG_ERROR("Time is invalid!");
        return;
    }

    int hours = 0;

    if (formatAp()) {
      hours = time.pmhour();

      if (time.hour() < 12)
	cbAP_->setCurrentIndex(0);
      else
	cbAP_->setCurrentIndex(1);
    } else
      hours = time.hour();

    int minutes = time.minute();
    int seconds = time.second();
    int millisecond = time.msec();

    sbhour_->setValue(hours);
    sbminute_->setValue(minutes);
    sbsecond_->setValue(seconds);

    if (formatMs()) {
        sbmillisecond_->setValue(millisecond);
    }
}

void WTimePicker::configure()
{
    WTemplate *container = dynamic_cast<WTemplate *>(implementation());
    container->bindWidget("hour", sbhour_);
    container->bindWidget("minute", sbminute_);
    container->bindWidget("second", sbsecond_);

    if (formatMs())
      container->bindWidget("millisecond", sbmillisecond_);
    else {
      container->takeWidget("millisecond");
      container->bindEmpty("millisecond");
    }

    if (formatAp()) {
      sbhour_->setRange(1, 12);
      container->bindWidget("ampm", cbAP_);
    } else {
      container->takeWidget("ampm");
      container->bindEmpty("ampm");
      sbhour_->setRange(0, 23);
    }  
}

void WTimePicker::createWidgets()
{
    sbhour_ = new WSpinBox();
    sbhour_->setWidth(70);
    sbhour_->setSingleStep(1);
    sbhour_->changed().connect(this, &WTimePicker::hourValueChanged);

    sbminute_ = new WSpinBox();
    sbminute_->setWidth(70);
    sbminute_->setRange(0, 59);
    sbminute_->setSingleStep(1);
    sbminute_->changed().connect(this, &WTimePicker::minuteValueChanged);

    sbsecond_ = new WSpinBox();
    sbsecond_->setWidth(70);
    sbsecond_->setRange(0, 59);
    sbsecond_->setSingleStep(1);
    sbsecond_->changed().connect(this, &WTimePicker::secondValueChanged);

    sbmillisecond_ = new WSpinBox();
    sbmillisecond_->setWidth(70);
    sbmillisecond_->setRange(0, 999);
    sbmillisecond_->setSingleStep(1);
    sbmillisecond_->changed().connect(this, &WTimePicker::msecValueChanged);

    cbAP_ = new WComboBox();
    cbAP_->setWidth(90);
    cbAP_->addItem("AM");
    cbAP_->addItem("PM");
    cbAP_->changed().connect(this, &WTimePicker::ampmValueChanged);
}

void WTimePicker::hourValueChanged()
{
    if (sbhour_->validate() == Wt::WValidator::Valid)
      selectionChanged_.emit();
}

void WTimePicker::minuteValueChanged()
{
    if (sbminute_->validate() == Wt::WValidator::Valid){
        selectionChanged_.emit();
    }
}

void WTimePicker::secondValueChanged()
{
    if (sbsecond_->validate() == Wt::WValidator::Valid){
        selectionChanged_.emit();
    }
}

void WTimePicker::msecValueChanged()
{
    if (sbmillisecond_->validate() == Wt::WValidator::Valid){
        selectionChanged_.emit();
    }
}

void WTimePicker::ampmValueChanged()
{
    if(cbAP_->validate() == Wt::WValidator::Valid){
        selectionChanged_.emit();
    }
}

bool WTimePicker::formatAp() const
{
    return WTime::usesAmPm(timeEdit_->format());
}

bool WTimePicker::formatMs() const
{
    WT_USTRING format = timeEdit_->format();

    return WTime::fromString(WTime(4, 5, 6, 123).toString(format),
			     format).msec() == 123;
}

} // end namespace Wt
