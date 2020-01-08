// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2016 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_AUTH_OAUTH_WIDGET_H_
#define WT_AUTH_OAUTH_WIDGET_H_

#include <Wt/WImage.h>

namespace Wt {
  namespace Auth {

class OAuthProcess;

class WT_API OAuthWidget : public WImage
{
public:
  OAuthWidget(const OAuthService& oAuthService);

  Signal<OAuthProcess *, Identity>& authenticated() { return authenticated_; }

private:
  std::unique_ptr<OAuthProcess> process_;
  Signal<OAuthProcess *, Identity> authenticated_;

  void oAuthDone(const Identity& identity);
};

  }
}

#endif // WT_AUTH_OAUTH_WIDGET_H_
