/*
 * Copyright (C) 2025 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "LocalStringExample.h"

#include <Wt/WString.h>
#include <Wt/WText.h>
#include <Wt/WGridLayout.h>

LocalStringExample::LocalStringExample()
{
  auto layout = setLayout(std::make_unique<Wt::WGridLayout>());

  // Will change depending on the locale.
  layout->addWidget(std::make_unique<Wt::WText>(Wt::WString::tr("LocalStringExample_baseText")), 0, 1);
  layout->addWidget(std::make_unique<Wt::WText>("Localized WString:"), 0, 0);

  // Will not change because it is literal.
  layout->addWidget(std::make_unique<Wt::WText>(Wt::WString("This string is literal. It will not change.")), 1, 1);
  layout->addWidget(std::make_unique<Wt::WText>("Literal WString:"), 1, 0);

  // Not defined in message_nl.xml. Will fallback to English if Dutch language is selected.
  layout->addWidget(std::make_unique<Wt::WText>(Wt::WString::tr("LocalStringExample_fallbackText")), 2, 1);
  layout->addWidget(std::make_unique<Wt::WText>("Localized WString not defined in Dutch (will fallback to English):"), 2, 0);

  // Is not defined anywhere. Will fallback to '??key??'.
  layout->addWidget(std::make_unique<Wt::WText>(Wt::WString::tr("undefined_message")), 3, 1);
  layout->addWidget(std::make_unique<Wt::WText>("Undefined localized WString:"), 3, 0);

  // Is not defined in message.xml. Will fallback to '??key??' when English is selected.
  layout->addWidget(std::make_unique<Wt::WText>(Wt::WString::tr("LocalStringExample_noFallbackText")), 4, 1);
  layout->addWidget(std::make_unique<Wt::WText>("Localized WString not defined in English (will fallback to '??key??' in English):"), 4, 0);
}