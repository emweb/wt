#include <Wt/WText.h>

SAMPLE_BEGIN(TextXHTML)
auto text
    = Wt::cpp14::make_unique<Wt::WText>("This is <b>XHTML</b> markup text. "
		    "It supports a safe subset of XHTML tags and "
		    "attributes, which have only decorative "
		    "functions.");

SAMPLE_END(return std::move(text))
