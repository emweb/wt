
#include "Wt/WApplication"

#include "Wt/WTime"
#include "Wt/WTimeValidator"
#include "Wt/WLocale"
#include "Wt/WLogger"
#include "Wt/WStringStream"


namespace Wt {

LOGGER("WTimeValidator");

WTimeValidator::WTimeValidator(WObject *parent)
  : WRegExpValidator(parent)
{
#ifndef WT_TARGET_JAVA
  setFormat(WLocale::currentLocale().timeFormat());
#else
  setFormat(WTime::defaultFormat());
#endif
}

WTimeValidator::WTimeValidator(const WT_USTRING& format, WObject *parent)
  : WRegExpValidator(parent)
{
  setFormat(format);
}

WTimeValidator::WTimeValidator(const WT_USTRING &format, const WTime &bottom,
                               const WTime &top, WObject *parent)
    : WRegExpValidator(parent),
      bottom_(bottom),
      top_(top)
{
    setFormat(format);
}

void WTimeValidator::setFormat(const WT_USTRING& format)
{
  if (format_ != format) {
    format_ = format;
    setRegExp(WTime::formatToRegExp(format).regexp);
    repaint();
  }
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

void WTimeValidator::setInvalidNotATimeText(const WString &text)
{
    notATimeText_ = text;
}

WString WTimeValidator::invalidNotATimeText() const
{
    if(!notATimeText_.empty()){
        WString s = notATimeText_;
        s.arg(format_);
        return s;
    } else
        return WString::tr("Wt.WTimeValidator.WrongFormat").arg(format_);
}

void WTimeValidator::setInvalidTooEarlyText(const WString &text)
{
    tooEarlyText_ = text;
    repaint();
}

WString WTimeValidator::invalidTooEarlyText() const
{
    if(!tooEarlyText_.empty()){
        WString s = tooEarlyText_;
        s.arg(bottom_.toString(format_)).arg(top_.toString(format_));
        return s;
    } else{
        if(bottom_.isNull())
            return WString();
        else{
            if(top_.isNull())
                return WString::tr("Wt.WTimeValidator.TimeTooEarly").arg(bottom_.toString(format_));
            else
                return WString::tr("Wt.WTimeValidator.WrongTimeRange").arg(bottom_.toString(format_))
                        .arg(top_.toString(format_));
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
    if(!tooLateText_.empty()){
        WString s = tooLateText_;
        s.arg(bottom_.toString(format_)).arg(top_.toString(format_));
        return s;
    } else{
        if(top_.isNull())
            return WString();
        else{
            if(bottom_.isNull())
                return WString::tr("Wt.WTimeValidator.TimeTooLate").arg(top_.toString(format_));
            else
                return WString::tr("Wt.WTimeValidator.WrongTimeRange").arg(bottom_.toString(format_))
                        .arg(top_.toString(format_));
        }
    }

}

WRegExpValidator::Result WTimeValidator::validate(const WString &input) const
{
    if(input.empty())
        return WRegExpValidator::validate(input);
    try{
        WTime t = WTime::fromString(input, format_);
        if(t.isValid()){
            if(!bottom_.isNull() && t < bottom_)
                return Result(Invalid, invalidTooEarlyText());
            if(!top_.isNull() && t > top_)
                return Result(Invalid, invalidTooLateText());
            return Result(Valid);
        }
    } catch(std::exception &e){
            LOG_WARN("validate(): " << e.what());
    }
    return Result(Invalid, invalidNotATimeText());
}

}
