#include "WTimePicker"

#include "WStringStream"
#include "WTemplate"
#include "WPushButton"
#include "WText"
#include "WIcon"


namespace Wt {

LOGGER("WTimePicker");

WTimePicker::WTimePicker(WContainerWidget *parent)
    : WCompositeWidget(parent), minuteStep_(1),
      selectionChanged_(this)
{
    init();
}

WTimePicker::WTimePicker(const WTime &time, WContainerWidget *parent)
    : WCompositeWidget(parent), minuteStep_(1)
{
    init(time);
}

void WTimePicker::init(const WTime &time)
{
    Wt::WStringStream text;
    text << "<table>"
            """<tr>"
            ""  "<th>${incrementHour}</th>"
            ""  "<th></th>"
            ""  "<th>${incrementMinute}</th>"
            """</tr>"
            """<tr>"
            ""  "<td valign=\"middle\" align=\"center\">${hourText}</td>"
            ""  "<td valign=\"middle\" align=\"center\">:</td>"
            ""  "<td valign=\"middle\" align=\"center\">${minuteText}</td>"
            """</tr>"
            """<tr>"
            ""  "<th>${decrementHour}</th>"
            ""  "<th></th>"
            ""  "<th>${decrementMinute}</th>"
            """</tr>"
            "</table>";

    WTemplate *impl = new WTemplate();
    setImplementation(impl);
    impl->setTemplateText(WString::fromUTF8(text.str(), XHTMLUnsafeText));

    WIcon::loadIconFont();
    WPushButton *incHourButton = new WPushButton();
    incHourButton->addStyleClass("fa fa-arrow-up");

    WPushButton *decHourButton = new WPushButton();
    decHourButton->addStyleClass("fa fa-arrow-down");

    WPushButton *incMinuteButton = new WPushButton();
    incMinuteButton->addStyleClass("fa fa-arrow-up");

    WPushButton *decMinuteButton = new WPushButton();
    decMinuteButton->addStyleClass("fa fa-arrow-down");

    hourText_ = new WText("0");
    hourText_->setInline(false);
    hourText_->setTextAlignment(AlignCenter);

    minuteText_ = new WText("00");
    minuteText_->setInline(false);
    minuteText_->setTextAlignment(AlignCenter);

    impl->bindWidget("incrementHour", incHourButton);
    impl->bindWidget("decrementHour", decHourButton);

    impl->bindWidget("hourText", hourText_);
    impl->bindWidget("minuteText", minuteText_);

    impl->bindWidget("incrementMinute", incMinuteButton);
    impl->bindWidget("decrementMinute", decMinuteButton);

    incHourButton->clicked().connect(this, &WTimePicker::incrementHours);
    decHourButton->clicked().connect(this, &WTimePicker::decrementHours);

    incMinuteButton->clicked().connect(this, &WTimePicker::incrementMinutes);
    decMinuteButton->clicked().connect(this, &WTimePicker::decrementMinutes);

}

WTime WTimePicker::time()
{
    int hours = 0, minutes = 0;

    try
    {
        hours = boost::lexical_cast<int>(hourText_->text().toUTF8());
        minutes = boost::lexical_cast<int>(minuteText_->text().toUTF8());
    }
    catch(const boost::bad_lexical_cast& ex)
    {
        LOG_ERROR("boost::bad_lexical_cast caught in WTimePicker::time()");
    }

    return WTime(hours, minutes);
}

void WTimePicker::setTime(const WTime& time)
{

    if(!time.isValid())
    {
        LOG_ERROR("Time is invalid!");
        return;
    }

    std::string hoursStr("0"), minutesStr("00");

    try
    {
        hoursStr = time.toString("hh").toUTF8();

        minutesStr = time.toString("mm").toUTF8();
    }
    catch(const boost::bad_lexical_cast& ex)
    {
        LOG_ERROR("boost::bad_lexical_cast caught in WTimePicker::time()");
    }

    hourText_->setText(hoursStr);
    minuteText_->setText(minutesStr);
}

void WTimePicker::incrementMinutes()
{
    std::string str = minuteText_->text().toUTF8();
    int curVal = 0;
    if(!str.empty())
    {
        try
        {
            curVal = boost::lexical_cast<int>(str);
        }
        catch(const boost::bad_lexical_cast& ex)
        {
		  LOG_ERROR("boost::bad_lexical_cast caught in WTimePicker::time()");
        }
    }

    if((curVal += minuteStep_) >= 60)
    {
        curVal -= 60;
    }

    try
    {
        str = boost::lexical_cast<std::string>(curVal);
        if(str.size() == 1) str = "0"+str;
    }
    catch(const boost::bad_lexical_cast& ex)
    {
        LOG_ERROR("boost::bad_lexical_cast caught in WTimePicker::time()");
    }

    minuteText_->setText(str);

    selectionChanged_.emit();
}

void WTimePicker::decrementMinutes()
{
    std::string str = minuteText_->text().toUTF8();
    int curVal = 0;
    if(!str.empty())
    {
        try
        {
            curVal = boost::lexical_cast<int>(str);
        }
        catch(const boost::bad_lexical_cast& ex)
        {
		  LOG_ERROR("boost::bad_lexical_cast caught in WTimePicker::time()");
        }
    }

    if((curVal -= minuteStep_) < 0)
    {
        curVal += 60;
    }

    try
    {
        str = boost::lexical_cast<std::string>(curVal);
        if(str.size() == 1) str = "0"+str;
    }
    catch(const boost::bad_lexical_cast& ex)
    {
	  LOG_ERROR("boost::bad_lexical_cast caught in WTimePicker::time()");
    }

    minuteText_->setText(str);

    selectionChanged_.emit();
}

void WTimePicker::incrementHours()
{
    std::string str = hourText_->text().toUTF8();
    int curVal = 0;
    if(!str.empty())
    {
        try {

            curVal = boost::lexical_cast<int>(str);
        }
        catch(const boost::bad_lexical_cast& ex)
        {
		  LOG_ERROR("boost::bad_lexical_cast caught in WTimePicker::time()");
        }
    }

    if((curVal + 1) < 24)
    {
        curVal++;
    }
    else
    {
        curVal -= 23;
    }

    try
    {
        str = boost::lexical_cast<std::string>(curVal);
        if(str.size() == 1) str = "0"+str;
    }
    catch(const boost::bad_lexical_cast& ex)
    {
		  LOG_ERROR("boost::bad_lexical_cast caught in WTimePicker::time()");
    }


    hourText_->setText(str);

    selectionChanged_.emit();
}

void WTimePicker::decrementHours()
{
    std::string str = hourText_->text().toUTF8();
    int curVal = 0;
    if(!str.empty())
    {
        try
        {
            curVal = boost::lexical_cast<int>(str);
        }
        catch(const boost::bad_lexical_cast& ex)
        {
		  LOG_ERROR("boost::bad_lexical_cast caught in WTimePicker::time()");
        }
    }

    if((curVal - 1) >= 0)
    {
        curVal--;
    }
    else
    {
        curVal += 23;
    }

    try
    {
        str = boost::lexical_cast<std::string>(curVal);
        if(str.size() == 1) str = "0"+str;
    }
    catch(const boost::bad_lexical_cast& ex)
    {
		  LOG_ERROR("boost::bad_lexical_cast caught in WTimePicker::time()");
    }

    hourText_->setText(str);

    selectionChanged_.emit();
}

} // end namespace Wt
