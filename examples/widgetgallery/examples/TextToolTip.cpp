#include <Wt/WText>

SAMPLE_BEGIN(TextToolTip)
Wt::WText *text
  = new Wt::WText("Some text", Wt::PlainText);

text->setToolTip("ToolTip",  Wt::XHTMLText);

SAMPLE_END(return text)
