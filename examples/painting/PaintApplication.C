/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WApplication.h>

#include "PaintExample.h"

using namespace Wt;

class PaintApplication: public WApplication
{
public:
  PaintApplication(const WEnvironment &env): WApplication(env) {
    setTitle("Paint example");
    
    useStyleSheet("painting.css");
    
    root()->addWidget(std::make_unique<PaintExample>());
  }
};

std::unique_ptr<WApplication> createApplication(const WEnvironment& env)
{
  return std::make_unique<PaintApplication>(env);
}

int main(int argc, char **argv)
{
  return WRun(argc, argv, &createApplication);
}
