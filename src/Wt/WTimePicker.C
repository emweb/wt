#include "WTimePicker.h"

#include "WStringStream.h"
#include "WTemplate.h"
#include "WLogger.h"
#include "WPushButton.h"
#include "WText.h"
#include "WIcon.h"
#include "WContainerWidget.h"
#include "WSpinBox.h"
#include "WTimeValidator.h"
#include "WTimeEdit.h"
#include "WJavaScriptSlot.h"

#include "WebUtils.h"

namespace Wt {

LOGGER("WTimePicker");

WTimePicker::WTimePicker(WTimeEdit *timeEdit)
  : timeEdit_(timeEdit),
    toggleAmPm_(2, this)
{
  init();
}

void WTimePicker::init(const WTime &time)
{
    WTemplate *container = new WTemplate();
    setImplementation(std::unique_ptr<WTemplate>(container));
    container->addStyleClass("form-inline");
    container->setTemplateText(tr("Wt.WTimePicker.template"));

    sbhour_ = container->bindWidget("hour", std::make_unique<WSpinBox>());
    sbhour_->setWidth(70);
    sbhour_->setSingleStep(1);
    sbhour_->changed().connect(this, &WTimePicker::hourValueChanged);

    sbminute_ = container->bindWidget("minute", std::make_unique<WSpinBox>());
    sbminute_->setWidth(70);
    sbminute_->setRange(0, 59);
    sbminute_->setSingleStep(1);
    sbminute_->changed().connect(this, &WTimePicker::minuteValueChanged);

    sbsecond_ = nullptr;
    container->bindEmpty("second");
    sbmillisecond_ = nullptr;
    container->bindEmpty("millisecond");
    cbAP_ = nullptr;
    container->bindEmpty("ampm");

    configure();
}

WTime WTimePicker::time() const
{
    int hours = 0, minutes = 0, seconds = 0, milliseconds = 0;

    try {
        hours = Utils::stoi(sbhour_->text().toUTF8());
        minutes = Utils::stoi(sbminute_->text().toUTF8());

        if (formatS())
          seconds = Utils::stoi(sbsecond_->text().toUTF8());

        if (formatMs())
            milliseconds = Utils::stoi(sbmillisecond_->text().toUTF8());
	if (formatAp()) {
	  if (cbAP_->currentIndex() == 1) {
	    if (hours != 12)
	      hours += 12;
	  } else
	    if (hours == 12)
	      hours = 0;
	}
    } catch (std::exception& e) {
        LOG_ERROR("stoi() std::exception in WTimePicker::time()");
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

    if (formatS())
      sbsecond_->setValue(seconds);

    if (formatMs()) {
        sbmillisecond_->setValue(millisecond);
    }
}

void WTimePicker::configure()
{
    WTemplate *container = dynamic_cast<WTemplate *>(implementation());

    if (formatS()) {
      sbsecond_ = container->bindWidget("second", std::make_unique<WSpinBox>());
      sbsecond_->setWidth(70);
      sbsecond_->setRange(0, 59);
      sbsecond_->setSingleStep(1);
      sbsecond_->changed().connect(this, &WTimePicker::secondValueChanged);

      sbsecond_->setWrapAroundEnabled(wrapAroundEnabled());
    } else {
      if (sbsecond_) {
	container->removeWidget("second");
	sbsecond_ = nullptr;
	container->bindEmpty("second");
      }
    }

    if (formatMs()) {
      if (!sbmillisecond_) {
	sbmillisecond_ = container->bindWidget("millisecond", std::make_unique<WSpinBox>());
	sbmillisecond_->setWidth(70);
	sbmillisecond_->setRange(0, 999);
	sbmillisecond_->setSingleStep(1);
	sbmillisecond_->changed().connect(this, &WTimePicker::msecValueChanged);
	
	sbmillisecond_->setWrapAroundEnabled(wrapAroundEnabled());
      }
    } else {
      if (sbmillisecond_) {
	container->removeWidget("millisecond");
	sbmillisecond_ = nullptr;
	container->bindEmpty("millisecond");
      }
    }

    if (formatAp()) {
      if (!cbAP_) {
	cbAP_ = container->bindWidget("ampm", std::make_unique<WComboBox>());
	cbAP_->setWidth(90);
	cbAP_->addItem("AM");
	cbAP_->addItem("PM");
	cbAP_->changed().connect(this, &WTimePicker::ampmValueChanged);
      }
      sbhour_->setRange(1, 12);
    } else {
      if (cbAP_) {
	container->removeWidget("ampm");
	cbAP_ = nullptr;
	container->bindEmpty("ampm");
      }
      sbhour_->setRange(0, 23);
    }

    if (cbAP_) {
      WStringStream jsValueChanged;

      jsValueChanged << "function(o,e,oldv,v){"
		     << "var obj = " << cbAP_->jsRef() << ";"
		     << "if(obj){"
		     << "if (v==12 && oldv==11) {"
		     << "obj.selectedIndex = (obj.selectedIndex + 1) % 2;"
		     << "}"
		     << "if (v==11 && oldv==12) {"
		     << "obj.selectedIndex = (obj.selectedIndex + 1) % 2;"
		     << "}"
		     << "}"
		     <<"}";

      toggleAmPm_.setJavaScript(jsValueChanged.str());
    } else {
      toggleAmPm_.setJavaScript("function(){}");
    }
}

void WTimePicker::setWrapAroundEnabled(bool enabled)
{
  sbhour_->setWrapAroundEnabled(enabled);
  sbminute_->setWrapAroundEnabled(enabled);
  if (sbsecond_)
    sbsecond_->setWrapAroundEnabled(enabled);
  if (sbmillisecond_)
    sbmillisecond_->setWrapAroundEnabled(enabled);
  if (enabled) {
    sbhour_->jsValueChanged().connect(toggleAmPm_);
  } else {
    sbhour_->jsValueChanged().disconnect(toggleAmPm_);
  }
}

bool WTimePicker::wrapAroundEnabled() const
{
  return sbhour_->wrapAroundEnabled();
}

void WTimePicker::hourValueChanged()
{
  if (sbhour_->validate() == ValidationState::Valid)
    selectionChanged_.emit();
}

void WTimePicker::minuteValueChanged()
{
  if (sbminute_->validate() == ValidationState::Valid)
    selectionChanged_.emit();
}

void WTimePicker::secondValueChanged()
{
  if (sbsecond_->validate() == ValidationState::Valid)
    selectionChanged_.emit();
}

void WTimePicker::msecValueChanged()
{
  if (sbmillisecond_->validate() == ValidationState::Valid)
    selectionChanged_.emit();
}

void WTimePicker::ampmValueChanged()
{
  if (cbAP_->validate() == ValidationState::Valid)
    selectionChanged_.emit();
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

bool WTimePicker::formatS() const
{
    WT_USTRING format = timeEdit_->format();

    return WTime::fromString(WTime(4, 5, 6, 123).toString(format),
			     format).second() == 6;
}

void WTimePicker::setHourStep(int step)
{
  sbhour_->setSingleStep(step);
}

int WTimePicker::hourStep() const
{
  return sbhour_->singleStep();
}

void WTimePicker::setMinuteStep(int step)
{
  sbminute_->setSingleStep(step);
}

int WTimePicker::minuteStep() const
{
  return sbminute_->singleStep();
}

void WTimePicker::setSecondStep(int step)
{
  if (sbsecond_)
    sbsecond_->setSingleStep(step);
}

int WTimePicker::secondStep() const
{
  if (sbsecond_)
    return sbsecond_->singleStep();
  else
    return 1;
}

void WTimePicker::setMillisecondStep(int step)
{
  if (sbmillisecond_)
    sbmillisecond_->setSingleStep(step);
}

int WTimePicker::millisecondStep() const
{
  if (sbmillisecond_)
    return sbmillisecond_->singleStep();
  else
    return 1;
}


} // end namespace Wt
