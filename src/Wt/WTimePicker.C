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

void WTimePicker::init(const WTime &time)
{
    WTemplate *container = new WTemplate();
    setImplementation(container);
    container->addStyleClass("form-inline");
    container->setTemplateText(tr("Wt.WTimePicker.template"));
    createWidgets();
    configure();
}

WTime WTimePicker::time()
{
    int hours = 0, minutes = 0, seconds = 0, milliseconds = 0;

    try {
        hours = boost::lexical_cast<int>(sbhour_->text().toUTF8());
        minutes = boost::lexical_cast<int>(sbminute_->text().toUTF8());
        seconds = boost::lexical_cast<int>(sbsecond_->text().toUTF8());
        if(formatMs())
            milliseconds = boost::lexical_cast<int>(sbmillisecond_->text().toUTF8());
    } catch(const boost::bad_lexical_cast& ex) {
        LOG_ERROR("boost::bad_lexical_cast caught in WTimePicker::time()");
    }
    if(formatMs())
        return WTime(hours, minutes, seconds, milliseconds);
    return WTime(hours, minutes, seconds);
}

void WTimePicker::setTime(const WTime& time)
{
    if(!time.isValid()) {
        LOG_ERROR("Time is invalid!");
        return;
    }

    std::string hoursStr("0"), minutesStr("00"), secondsStr("00"), millisecondStr("000");

    try {
        hoursStr = time.toString("hh").toUTF8();
        if(formatAp()){
            int hours = boost::lexical_cast<int>(hoursStr);
            if(hours > 12){
                hours -= 12;
                hoursStr = boost::lexical_cast<std::string>(hours);
            }
        }
        minutesStr = time.toString("mm").toUTF8();
		secondsStr = time.toString("ss").toUTF8();
        millisecondStr = time.toString("z").toUTF8();
    } catch(const boost::bad_lexical_cast& ex) {
        LOG_ERROR("boost::bad_lexical_cast caught in WTimePicker::time()");
    }

    sbhour_->setText(hoursStr);
    sbminute_->setText(minutesStr);
    sbsecond_->setText(secondsStr);

    if(formatMs()){
        sbmillisecond_->setText(millisecondStr);
    }

}

void WTimePicker::setTime(const WTime &time, std::string &ampm)
{
    setTime(time);
    if(ampm == "AM")
        cbAP_->setCurrentIndex(0);
    else
        cbAP_->setCurrentIndex(1);
}

std::string WTimePicker::ampm() const
{
    return cbAP_->currentText().toUTF8();
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
      sbhour_->setRange(0, 13);
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
    cbAP_->setWidth(70);
    cbAP_->addItem("AM");
    cbAP_->addItem("PM");
    cbAP_->changed().connect(this, &WTimePicker::ampmValueChanged);
}

void WTimePicker::hourValueChanged()
{
    if (sbhour_->validate() == Wt::WValidator::Valid){
      if (formatAp()) {
	if (sbhour_->value() == 13) {
	  sbhour_->setValue(1);
	  cbAP_->setCurrentIndex(1 - cbAP_->currentIndex());
        } else if (sbhour_->value() == 0) {
	  sbhour_->setValue(12);
	  cbAP_->setCurrentIndex(1 - cbAP_->currentIndex());
	}
      }

      selectionChanged_.emit();
    }
}

void WTimePicker::minuteValueChanged()
{
    if(sbminute_->validate() == Wt::WValidator::Valid){
        selectionChanged_.emit();
    }
}

void WTimePicker::secondValueChanged()
{
    if(sbsecond_->validate() == Wt::WValidator::Valid){
        selectionChanged_.emit();
    }
}

void WTimePicker::msecValueChanged()
{
    if(sbmillisecond_->validate() == Wt::WValidator::Valid){
        selectionChanged_.emit();
    }
}

void WTimePicker::ampmValueChanged()
{
    if(cbAP_->validate() == Wt::WValidator::Valid){
        selectionChanged_.emit();
    }
}

bool WTimePicker::formatAp()
{
    std::string format = timeEdit_->format().toUTF8();
    if(!format.empty())
        if(format.find("A") != std::string::npos || format.find("a") != std::string::npos)
            return true;
    return false;
}

bool WTimePicker::formatMs()
{
    std::string format = timeEdit_->format().toUTF8();
    if(!format.empty())
      if(format.find("z") != std::string::npos || format.find("Z") != std::string::npos)
	return true;
    return false;
}

} // end namespace Wt
