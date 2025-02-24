/*
 * Copyright (C) 2025 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "RefreshExamples.h"

#include <Wt/WString.h>
#include <Wt/WText.h>
#include <Wt/WGridLayout.h>

RefreshExamples::RefreshExamples()
{
  auto layout = setLayout(std::make_unique<Wt::WGridLayout>());

  localWString_ = Wt::WString::tr("ConcatenationExamples_baseText");

  literalWString_ = Wt::WString("Literal WString");

  str_ = "std::string";

  concatStr_ = layout->addWidget(std::make_unique<Wt::WText>(localWString_ + str_), 0, 1);
  layout->addWidget(std::make_unique<Wt::WText>("Localized WString concatenated with a std::string:"), 0, 0);

  concatLiteral_ = layout->addWidget(std::make_unique<Wt::WText>(localWString_ + literalWString_), 1, 1);
  layout->addWidget(std::make_unique<Wt::WText>("Localized WString concatenated with a literal WString:"), 1, 0);

  concatLocal_ = layout->addWidget(std::make_unique<Wt::WText>(localWString_ + localWString_), 2, 1);
  layout->addWidget(std::make_unique<Wt::WText>("Localized WString concatenated with another Localized WString:"), 2, 0);

  layout->addWidget(std::make_unique<Wt::WText>(localWString_), 3, 1);
  layout->addWidget(std::make_unique<Wt::WText>("Just a Localized WString (no need to be updated in refresh()):"), 3, 0);
}

void RefreshExamples::refresh()
{
  concatStr_->setText(localWString_ + str_);
  concatLiteral_->setText(localWString_ + literalWString_);
  concatLocal_->setText(localWString_ + localWString_);

  Wt::WContainerWidget::refresh();
}