// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef CODER_APPLICATION_H_
#define CODER_APPLICATION_H_

#include <Wt/WApplication.h>

class CoderApplication : public Wt::WApplication
{
public:
  CoderApplication(const Wt::WEnvironment& environment);

private:
  void handlePathChange();
  void createUI(const std::string& path);
};

#endif // CODER_APPLICATION_H_
