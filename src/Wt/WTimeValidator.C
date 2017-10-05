
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

void WTimeValidator::setInvalidNotATimeText(const WString &text)
{
    notATimeText_ = text;
}

WString WTimeValidator::invalidNotATimeText() const
{
    if(!notATimeText_.empty()){
        WString s = notATimeText_;
        s.arg(formats_[0]);
        return s;
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
    if(!tooEarlyText_.empty()){
        WString s = tooEarlyText_;
        s.arg(bottom_.toString(formats_[0])).arg(top_.toString(formats_[0]));
        return s;
    } else{
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
    if(!tooLateText_.empty()){
        WString s = tooLateText_;
        s.arg(bottom_.toString(formats_[0])).arg(top_.toString(formats_[0]));
        return s;
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

    js << ',' << invalidBlankText().jsStringLiteral()
       << ',' << invalidNotATimeText().jsStringLiteral()
       << ',' << invalidTooEarlyText().jsStringLiteral()
       << ',' << invalidTooLateText().jsStringLiteral()
       << ");";

    return js.str();
}


}
