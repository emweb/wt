/*
 * Copyright (C) 2025 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WContainerWidget.h>

class RefreshExamples : public Wt::WContainerWidget
{
public:
RefreshExamples();

void refresh() override;

private:
  Wt::WString localWString_, literalWString_;
  std::string str_;

  Wt::WText *concatStr_, *concatLiteral_, *concatLocal_;
};