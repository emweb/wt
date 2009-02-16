/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WApplication>

#include "PaintExample.h"

using namespace Wt;

class PaintApplication: public WApplication
{
public:
  PaintApplication(const WEnvironment &env): WApplication(env) {
    setTitle("Paint example");
    
    useStyleSheet("painting.css");
    
    new PaintExample(root());
  }
};

WApplication *createApplication(const WEnvironment& env)
{
  return new PaintApplication(env);
}

int main(int argc, char **argv)
{
  return WRun(argc, argv, &createApplication);
}
