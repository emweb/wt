#include <Wt/WText>

SAMPLE_BEGIN(TextXHTML)
Wt::WText *text
    = new Wt::WText("This is <b>XHTML</b> markup text. "
		    "It supports a safe subset of XHTML tags and "
		    "attributes, which have only decorative "
		    "functions.");

SAMPLE_END(return text)
