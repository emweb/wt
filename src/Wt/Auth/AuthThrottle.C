#include "AuthThrottle.h"

#include "Wt/WApplication.h"
#include "Wt/WLogger.h"
#include "Wt/WStringStream.h"

#ifndef WT_DEBUG_JS
#include "js/AuthThrottle.min.js"
#endif

namespace Wt {
  LOGGER("Auth.AuthThrottle");
  namespace Auth {
int AuthThrottle::delayForNextAttempt(const User& user) const
{
  int delay = 0;
  int throttlingNeeded = getAuthenticationThrottle(user.failedLoginAttempts());

  if (throttlingNeeded) {
    WDateTime t = user.lastLoginAttempt();
    int diff = t.secsTo(WDateTime::currentDateTime());

    if (diff < throttlingNeeded) {
      delay = throttlingNeeded - diff;
    }
  }

  if (delay > 0) {
    LOG_SECURE("delayForNextAttempt(): " << delay  << " seconds for user: " << user.id());
  }
  return delay;
}

int AuthThrottle::getAuthenticationThrottle(int failedAttempts) const
{
  switch (failedAttempts) {
    case 0:
      return 0;
    case 1:
      return 1;
    case 2:
      return 5;
    case 3:
      return 10;
    default:
      return 25;
  }
}


void AuthThrottle::initializeThrottlingMessage(WInteractWidget* button)
{
  WApplication *app = WApplication::instance();
  LOAD_JAVASCRIPT(app, "js/AuthThrottle.js", "AuthThrottle", wtjs1);

  button->setJavaScriptMember(" AuthThrottle",
                              "new " WT_CLASS ".AuthThrottle(" WT_CLASS ","
                              + button->jsRef() + ","
                              + WString::tr("Wt.Auth.throttle-retry")
                              .jsStringLiteral()
                              + ");");
}

void AuthThrottle::updateThrottlingMessage(WInteractWidget* button, int delay)
{
  WStringStream s;
  s << button->jsRef() << ".wtThrottle.reset("
    << delay << ");";

  button->doJavaScript(s.str());
}
  }
}
