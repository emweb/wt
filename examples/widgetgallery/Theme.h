// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2022 Emweb bv, Herent, Belgium
 *
 * See the LICENSE file for terms of use.
 */

#ifndef THEME_H_
#define THEME_H_

#include <Wt/WBootstrap5Theme.h>
#include <Wt/WLinkedCssStyleSheet.h>

#include <vector>

class Theme : public Wt::WBootstrap5Theme {
public:
  explicit Theme(const std::string& name);

  std::vector<Wt::WLinkedCssStyleSheet> styleSheets() const override;

private:
  std::string name_;
};

#endif // THEME_H_
