/*
 * Copyright (C) 2025 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "FormatExample.h"

#include <Wt/WString.h>
#include <Wt/WText.h>
#include <Wt/WGridLayout.h>

FormatExample::FormatExample()
{
  auto layout = setLayout(std::make_unique<Wt::WGridLayout>());

  Wt::WString localWString = Wt::WString::tr("FormatExample_baseText");

  Wt::WString literalWString = Wt::WString("Literal WString");

  std::string str = "std::string";

  // We need to copy localWString before calling arg() because it modifies the WString.

  layout->addWidget(std::make_unique<Wt::WText>(Wt::WString(localWString).arg(str)), 0, 1);
  layout->addWidget(std::make_unique<Wt::WText>("Localized WString formatted with a std::string:"), 0, 0);

  layout->addWidget(std::make_unique<Wt::WText>(Wt::WString(localWString).arg(literalWString)), 1, 1);
  layout->addWidget(std::make_unique<Wt::WText>("Localized WString formatted with a literal WString:"), 1, 0);

  layout->addWidget(std::make_unique<Wt::WText>(Wt::WString(localWString).arg(
    // We remove the placeholder by passing an empty string.
    Wt::WString(localWString).arg(""))), 2, 1);
  layout->addWidget(std::make_unique<Wt::WText>("Localized WString formatted with another Localized WString:"), 2, 0);

  layout->addWidget(std::make_unique<Wt::WText>(localWString), 3, 1);
  layout->addWidget(std::make_unique<Wt::WText>("Just a Localized WString:"), 3, 0);

  Wt::WString literalWString2 = literalWString + " {1}";

  // This will not change with the new locale because it is a literal WString.
  layout->addWidget(std::make_unique<Wt::WText>(literalWString2.arg(
    // We remove the placeholder by passing an empty string.
    Wt::WString(localWString).arg(""))), 4, 1);
  layout->addWidget(std::make_unique<Wt::WText>("Literal WString formatted with a Localized WString using:"), 4, 0);

  // literalWString2 is now modified.
  layout->addWidget(std::make_unique<Wt::WText>(literalWString2), 5, 1);
  layout->addWidget(std::make_unique<Wt::WText>("Same WString as above, showing that .arg() modifies the original WString:"), 5, 0);

}