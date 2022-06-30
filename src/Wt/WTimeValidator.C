
#include "Wt/WApplication.h"

#include "Wt/WTime.h"
#include "Wt/WTimeValidator.h"
#include "Wt/WLocale.h"
#include "Wt/WLogger.h"
#include "Wt/WStringStream.h"
#include "Wt/WWebWidget.h"

#ifndef WT_DEBUG_JS
#include "js/WTimeValidator.min.js"
#endif


namespace Wt {

LOGGER("WTimeValidator");

WTimeValidator::WTimeValidator()
{
#ifndef WT_TARGET_JAVA
  setFormat(WLocale::currentLocale().timeFormat());
#else
  setFormat(WTime::defaultFormat());
#endif
}

WTimeValidator::WTimeValidator(const WT_USTRING& format)
{
  setFormat(format);
}

WTimeValidator::WTimeValidator(const WT_USTRING &format, const WTime &bottom,
                               const WTime &top)
    : bottom_(bottom),
      top_(top)
{
  setFormat(format);
}

void WTimeValidator::setFormat(const WT_USTRING& format)
{
  if (formats_.empty() || formats_[0] != format){
      formats_.clear();
      formats_.push_back(format);
      repaint();
  }
}

void WTimeValidator::setFormats(const std::vector<WT_USTRING> &formats)
{
    formats_ = formats;
    repaint();
}

void WTimeValidator::setBottom(const WTime &bottom)
{
    if(bottom_ != bottom){
        bottom_ = bottom;
        repaint();
    }
}

void WTimeValidator::setTop(const WTime &top)
{
    if(top_ != top){
        top_ = top;
        repaint();
    }
}

void WTimeValidator::setStep(const std::chrono::seconds& step)
{
  if (step < std::chrono::seconds(0)) {
    LOG_ERROR("WTimeValidator::setStep(): ignoring call, value should not be negative");
    return;
  }

  if (step_ != step) {
    step_ = step;
    repaint();
  }
}

void WTimeValidator::setInvalidNotATimeText(const WString &text)
{
    notATimeText_ = text;
}

WString WTimeValidator::invalidNotATimeText() const
{
    if (!notATimeText_.empty()) {
        return WString(notATimeText_).arg(formats_[0]);
    } else
        return WString::tr("Wt.WTimeValidator.WrongFormat").arg(formats_[0]);
}

void WTimeValidator::setInvalidTooEarlyText(const WString &text)
{
    tooEarlyText_ = text;
    repaint();
}

WString WTimeValidator::invalidTooEarlyText() const
{
    if (!tooEarlyText_.empty()) {
        return WString(tooEarlyText_).arg(bottom_.toString(formats_[0])).arg(top_.toString(formats_[0]));
    } else {
        if(bottom_.isNull())
            return WString();
        else{
            if(top_.isNull())
                return WString::tr("Wt.WTimeValidator.TimeTooEarly").arg(bottom_.toString(formats_[0]));
            else
                return WString::tr("Wt.WTimeValidator.WrongTimeRange").arg(bottom_.toString(formats_[0]))
                        .arg(top_.toString(formats_[0]));
        }
    }
}

void WTimeValidator::setInvalidTooLateText(const WString &text)
{
    tooLateText_ = text;
    repaint();
}

WString WTimeValidator::invalidTooLateText() const
{
    if (!tooLateText_.empty()) {
        return WString(tooLateText_).arg(bottom_.toString(formats_[0])).arg(top_.toString(formats_[0]));
    } else{
        if(top_.isNull())
            return WString();
        else{
            if(bottom_.isNull())
                return WString::tr("Wt.WTimeValidator.TimeTooLate").arg(top_.toString(formats_[0]));
            else
                return WString::tr("Wt.WTimeValidator.WrongTimeRange").arg(bottom_.toString(formats_[0]))
                        .arg(top_.toString(formats_[0]));
        }
    }

}

void WTimeValidator::setInvalidWrongStepText(const WString &text)
{
  wrongStepText_ = text;
  repaint();
}

WString WTimeValidator::invalidWrongStepText() const
{
  if (step_ <= std::chrono::seconds(0))
    return WString::Empty;
  else if (!wrongStepText_.empty())
    return wrongStepText_;
  else if (step_.count() % 60 == 0)
    return WString::tr("Wt.WTimeValidator.WrongStep-minutes").arg(step_.count() / 60);
  else if (step_.count() % 1 == 0)
    return WString::tr("Wt.WTimeValidator.WrongStep-seconds").arg(step_.count() / 1);
  else
    return WString::tr("Wt.WTimeValidator.WrongStep");
}

WValidator::Result WTimeValidator::validate(const WT_USTRING &input) const
{
    if (input.empty())
        return WValidator::validate(input);
    for (unsigned i = 0; i < formats_.size(); i++){
        try {
            WTime t = WTime::fromString(input, formats_[i]);
            if(t.isValid()){
                if(!bottom_.isNull() && t < bottom_)
                  return Result(ValidationState::Invalid,
                                invalidTooEarlyText());
                if(!top_.isNull() && t > top_)
                  return Result(ValidationState::Invalid,
                                invalidTooLateText());
                if (step_ > std::chrono::seconds(0)) {
                  WTime start = !bottom_.isNull() ? bottom_ : WTime(0, 0);
                  long secs = start.secsTo(t);
                  if (secs % step_.count() != 0)
                    return Result(ValidationState::Invalid, invalidWrongStepText());
                }
                return Result(ValidationState::Valid);
            }
        } catch (std::exception &e){
          LOG_WARN("validate(): " << e.what());
        }
    }
    return Result(ValidationState::Invalid, invalidNotATimeText());
}

void WTimeValidator::loadJavaScript(WApplication *app)
{
    LOAD_JAVASCRIPT(app, "js/WTimeValidator.js", "WTimeValidator", wtjs1);
}

std::string WTimeValidator::javaScriptValidate() const
{
    loadJavaScript(WApplication::instance());

    WStringStream js;

    js << "new " WT_CLASS ".WTimeValidator("
       << isMandatory()
       << ",[";

    for(unsigned i = 0; i < formats_.size(); ++i){
        WTime::RegExpInfo r = WTime::formatToRegExp(formats_[i]);

        if(i != 0)
            js << ',';
        js << "{"
           << "regexp:" << WWebWidget::jsStringLiteral(r.regexp) << ','
           << "getHour:function(results){" << r.hourGetJS << ";},"
           << "getMinutes:function(results){" << r.minuteGetJS << ";},"
           << "getSeconds:function(results){" << r.secGetJS << ";},"
           << "getMilliseconds:function(results){" << r.msecGetJS << ";},"
           << "}";
    }
    js << "],";

    if(!bottom_.isNull())
        js << "new Date(0,0,0,"
           << bottom_.hour() << "," << bottom_.minute() << ","
           << bottom_.second() << "," << bottom_.msec()
           << ")";
    else
        js << "null";
    js << ',';

    if(!top_.isNull())
        js << "new Date(0,0,0,"
           << top_.hour() << "," << top_.minute() << ","
           << top_.second() << "," << top_.msec()
           << ")";
    else
        js << "null";
    js << ',';

    if (step_ > std::chrono::seconds(0))
      js << static_cast<int>(step_.count());
    else
      js << "null";

    js << ',' << invalidBlankText().jsStringLiteral()
       << ',' << invalidNotATimeText().jsStringLiteral()
       << ',' << invalidTooEarlyText().jsStringLiteral()
       << ',' << invalidTooLateText().jsStringLiteral()
       << ',' << invalidWrongStepText().jsStringLiteral()
       << ");";

    return js.str();
}


}
