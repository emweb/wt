
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

void WTimeValidator::setFormat(const WT_USTRING& format)
{
  if (format_ != format) {
    format_ = format;
    setRegExp(WTime::formatToRegExp(format).regexp);
    repaint();
  }
}


}
