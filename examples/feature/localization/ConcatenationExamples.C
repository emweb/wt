/*
 * Copyright (C) 2025 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "ConcatenationExamples.h"

#include <Wt/WString.h>
#include <Wt/WText.h>
#include <Wt/WGridLayout.h>

ConcatenationExamples::ConcatenationExamples()
{
  auto layout = setLayout(std::make_unique<Wt::WGridLayout>());

  Wt::WString localWString = Wt::WString::tr("ConcatenationExamples_baseText");

  Wt::WString literalWString = Wt::WString("Literal WString");

  std::string str = "std::string";

  // Will not change.
  layout->addWidget(std::make_unique<Wt::WText>(localWString + str), 0, 1);
  layout->addWidget(std::make_unique<Wt::WText>("Localized WString concatenated with a std::string:"), 0, 0);

  // Will not change.
  layout->addWidget(std::make_unique<Wt::WText>(localWString + literalWString), 1, 1);
  layout->addWidget(std::make_unique<Wt::WText>("Localized WString concatenated with a literal WString:"), 1, 0);

  // Will not change.
  layout->addWidget(std::make_unique<Wt::WText>(localWString + localWString), 2, 1);
  layout->addWidget(std::make_unique<Wt::WText>("Localized WString concatenated with another Localized WString:"), 2, 0);

  // Will change.
  layout->addWidget(std::make_unique<Wt::WText>(localWString), 3, 1);
  layout->addWidget(std::make_unique<Wt::WText>("Just a Localized WString:"), 3, 0);

  // localWString becomes literal due to += operator
  localWString += literalWString;

  // Will not change
  layout->addWidget(std::make_unique<Wt::WText>(localWString), 4, 1);
  layout->addWidget(std::make_unique<Wt::WText>("Localized WString concatenated with a literal WString using += :"), 4, 0);
}