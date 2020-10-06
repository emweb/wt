#include <Wt/WText.h>

SAMPLE_BEGIN(TextPlain)
auto text
  = std::make_unique<Wt::WText>("This is an example of plain text. "
		  "Any contained special XHTML characters, "
		  "such as \"<\" and \">\", "
                  "are automatically escaped.", Wt::TextFormat::Plain);

SAMPLE_END(return std::move(text))
