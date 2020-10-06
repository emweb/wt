#include <Wt/WText.h>

SAMPLE_BEGIN(TextToolTip)
auto text
  = std::make_unique<Wt::WText>("Some text", Wt::TextFormat::Plain);

text->setToolTip("ToolTip",  Wt::TextFormat::XHTML);

SAMPLE_END(return std::move(text))
