#include <Wt/WText>

SAMPLE_BEGIN(TextPlain)
Wt::WText *text
  = new Wt::WText("This is an example of plain text. "
		  "Any contained special XHTML characters, "
		  "such as \"<\" and \">\", "
		  "are automatically escaped.", Wt::PlainText);

SAMPLE_END(return text)
