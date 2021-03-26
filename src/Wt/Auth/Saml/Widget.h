// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2021 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_AUTH_SAML_WIDGET_H_
#define WT_AUTH_SAML_WIDGET_H_

#include <Wt/WImage.h>

namespace Wt {
  namespace Auth {
    namespace Saml {

class Process;

#ifdef WT_TARGET_JAVA
/*! \class Widget Wt/Auth/Saml/Widget.h
 *  \brief A %SAML widget.
 */
#endif // WT_TARGET_JAVA
class WT_API Widget : public WImage {
public:
  explicit Widget(const Service &samlService);

  Signal<Process *, Identity> &authenticated() { return authenticated_; }

private:
  std::unique_ptr<Process> process_;
  Signal<Process *, Identity> authenticated_;

  void samlDone(const Identity &identity);
};

    }
  }
}

#endif // WT_AUTH_SAML_WIDGET_H_
