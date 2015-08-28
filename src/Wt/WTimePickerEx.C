#include "WTimePickerEx"

#include "WStringStream"
#include "WTemplate"
#include "WPushButton"
#include "WText"
#include "WIcon"

namespace Wt {

LOGGER("WTimePickerEx");

WTimePickerEx::WTimePickerEx(WContainerWidget *parent)
    : WCompositeWidget(parent),
      hourStep_(1), minuteStep_(1), secondStep_(1), millisecondStep_(1),
      selectionChanged_(this)
{
    init();
}

WTimePickerEx::WTimePickerEx(const WTime &time, WContainerWidget *parent)
    : WCompositeWidget(parent),
      hourStep_(1), minuteStep_(1), secondStep_(1), millisecondStep_(1)
{
    init(time);
}

void WTimePickerEx::init(const WTime &time)
{
    Wt::WStringStream text;
    text << "<table>"
            """<tr>"
            ""  "<th>${incrementHour}</th>"
            ""  "<th></th>"
            ""  "<th>${incrementMinute}</th>"
            ""  "<th></th>"
            ""  "<th>${incrementSecond}</th>"
            ""  "<th></th>"
            ""  "<th>${incrementMillisecond}</th>"
            """</tr>"
            """<tr>"
            ""  "<td valign=\"middle\" align=\"center\">${hourText}</td>"
            ""  "<td valign=\"middle\" align=\"center\">:</td>"
            ""  "<td valign=\"middle\" align=\"center\">${minuteText}</td>"
            ""  "<td valign=\"middle\" align=\"center\">:</td>"
            ""  "<td valign=\"middle\" align=\"center\">${secondText}</td>"
            ""  "<td valign=\"middle\" align=\"center\">.</td>"
            ""  "<td valign=\"middle\" align=\"center\">${millisecondText}</td>"
            """</tr>"
            """<tr>"
            ""  "<th>${decrementHour}</th>"
            ""  "<th></th>"
            ""  "<th>${decrementMinute}</th>"
            ""  "<th></th>"
            ""  "<th>${decrementSecond}</th>"
            ""  "<th></th>"
            ""  "<th>${decrementMillisecond}</th>"
            """</tr>"
            "</table>";

    WTemplate *impl = new WTemplate();
    setImplementation(impl);
    impl->setTemplateText(WString::fromUTF8(text.str(), XHTMLUnsafeText));

    WIcon::loadIconFont();
    WPushButton *incHourButton = new WPushButton();
    incHourButton->addStyleClass("fa fa-arrow-up");
    incHourButton->setToolTip("Add Hours");

    WPushButton *decHourButton = new WPushButton();
    decHourButton->addStyleClass("fa fa-arrow-down");
    decHourButton->setToolTip("Subtract Hours");

    WPushButton *incMinuteButton = new WPushButton();
    incMinuteButton->addStyleClass("fa fa-arrow-up");
    incMinuteButton->setToolTip("Add Minutes");

    WPushButton *decMinuteButton = new WPushButton();
    decMinuteButton->addStyleClass("fa fa-arrow-down");
    decMinuteButton->setToolTip("Subtract Minutes");

    WPushButton *incSecondButton = new WPushButton();
    incSecondButton->addStyleClass("fa fa-arrow-up");
    incSecondButton->setToolTip("Add Seconds");

    WPushButton *decSecondButton = new WPushButton();
    decSecondButton->addStyleClass("fa fa-arrow-down");
    decSecondButton->setToolTip("Subtract Seconds");

    WPushButton *incMillisecondButton = new WPushButton();
    incMillisecondButton->addStyleClass("fa fa-arrow-up");
    incMillisecondButton->setToolTip("Add Milliseconds");

    WPushButton *decMillisecondButton = new WPushButton();
    decMillisecondButton->addStyleClass("fa fa-arrow-down");
    decMillisecondButton->setToolTip("Subtract Milliseconds");

    hourText_ = new WText("0");
    hourText_->setInline(false);
    hourText_->setTextAlignment(AlignCenter);

    minuteText_ = new WText("00");
    minuteText_->setInline(false);
    minuteText_->setTextAlignment(AlignCenter);

    secondText_ = new WText("00");
    secondText_->setInline(false);
    secondText_->setTextAlignment(AlignCenter);

    millisecondText_ = new WText("000");
    millisecondText_->setInline(false);
    millisecondText_->setTextAlignment(AlignCenter);

    impl->bindWidget("hourText", hourText_);
    impl->bindWidget("minuteText", minuteText_);
    impl->bindWidget("secondText", secondText_);
    impl->bindWidget("millisecondText", millisecondText_);

    impl->bindWidget("incrementHour", incHourButton);
    impl->bindWidget("decrementHour", decHourButton);

    impl->bindWidget("incrementMinute", incMinuteButton);
    impl->bindWidget("decrementMinute", decMinuteButton);

    impl->bindWidget("incrementSecond", incSecondButton);
    impl->bindWidget("decrementSecond", decSecondButton);

    impl->bindWidget("incrementMillisecond", incMillisecondButton);
    impl->bindWidget("decrementMillisecond", decMillisecondButton);

    incHourButton->clicked().connect(this, &WTimePickerEx::incrementHours);
    decHourButton->clicked().connect(this, &WTimePickerEx::decrementHours);

    incMinuteButton->clicked().connect(this, &WTimePickerEx::incrementMinutes);
    decMinuteButton->clicked().connect(this, &WTimePickerEx::decrementMinutes);

    incSecondButton->clicked().connect(this, &WTimePickerEx::incrementSeconds);
    decSecondButton->clicked().connect(this, &WTimePickerEx::decrementSeconds);

    incMillisecondButton->clicked().connect(this, &WTimePickerEx::incrementMilliseconds);
    decMillisecondButton->clicked().connect(this, &WTimePickerEx::decrementMilliseconds);
}

WTime WTimePickerEx::time()
{
   unsigned int hours = 0, minutes = 0, seconds = 0, milliseconds = 0;

    try
    {
        hours = boost::lexical_cast<unsigned int>(hourText_->text().toUTF8());
        minutes = boost::lexical_cast<unsigned int>(minuteText_->text().toUTF8());
        seconds = boost::lexical_cast<unsigned int>(secondText_->text().toUTF8());
        milliseconds = boost::lexical_cast<unsigned int>(millisecondText_->text().toUTF8());
    }
    catch(const boost::bad_lexical_cast& ex)
    {
        LOG_ERROR("boost::bad_lexical_cast caught in WTimePickerEx::time()");
    }

    return WTime(hours, minutes, seconds, milliseconds);
}

void WTimePickerEx::setTime(const WTime& time)
{

    if(!time.isValid())
    {
        LOG_ERROR("Time is invalid!");
        return;
    }

    std::string hoursStr("0"), minutesStr("00"), secondsStr("00"), millisecondsStr("000");

    try
    {
        hoursStr = time.toString("hh").toUTF8();
        minutesStr = time.toString("mm").toUTF8();
        secondsStr = time.toString("ss").toUTF8();
        millisecondsStr = time.toString("msms").toUTF8();
    }
    catch(const boost::bad_lexical_cast& ex)
    {
        LOG_ERROR("boost::bad_lexical_cast caught in WTimePickerEx::time()");
    }

    hourText_->setText(hoursStr);
    minuteText_->setText(minutesStr);
    secondText_->setText(secondsStr);
    millisecondText_->setText(millisecondsStr);
}

void WTimePickerEx::changeTime(WText* _timeText, int increment, int rollover)
{
    std::string str = _timeText->text().toUTF8();
    int curVal = 0;
    if(!str.empty())
    {
        try
        {
            curVal = boost::lexical_cast<int>(str);
        }
        catch(const boost::bad_lexical_cast& ex)
        {
                  LOG_ERROR("boost::bad_lexical_cast caught in WTimePickerEx::time()");
        }
    }

    if((curVal += increment) < 0)
    {
        curVal += rollover;
    }
    else if(curVal >= rollover)
    {
        curVal -= rollover;
    }

    try
    {
        str = boost::lexical_cast<std::string>(curVal);
        if(str.size() == 1)
	{
		if(rollover < 100) str = "0"+str;
		else str = "00"+str;
	}
	else if((str.size() == 2) && (rollover > 100))
	{
		str = "0"+str;
	}
    }
    catch(const boost::bad_lexical_cast& ex)
    {
          LOG_ERROR("boost::bad_lexical_cast caught in WTimePickerEx::time()");
    }

    _timeText->setText(str);

    selectionChanged_.emit();
}

void WTimePickerEx::incrementHours()
{
        changeTime(hourText_, hourStep_, 24);
}

void WTimePickerEx::decrementHours()
{
        changeTime(hourText_, -hourStep_, 24);
}

void WTimePickerEx::incrementMinutes()
{
	changeTime(minuteText_, minuteStep_, 60);
}

void WTimePickerEx::decrementMinutes()
{
        changeTime(minuteText_, -minuteStep_, 60);
}

void WTimePickerEx::incrementSeconds()
{
        changeTime(secondText_, secondStep_, 60);
}

void WTimePickerEx::decrementSeconds()
{
        changeTime(secondText_, -secondStep_, 60);
}

void WTimePickerEx::incrementMilliseconds()
{
        changeTime(millisecondText_, millisecondStep_, 1000);
}

void WTimePickerEx::decrementMilliseconds()
{
        changeTime(millisecondText_, -millisecondStep_, 1000);
}

} // end namespace Wt
