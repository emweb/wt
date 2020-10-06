#include <Wt/WText.h>

SAMPLE_BEGIN(TextXSS)
auto text
    = std::make_unique<Wt::WText>("<p>This XHTML text contains JavaScript, "
		    "wich is filtered by the XSS filter.</p>"
		    "<script>alert(\"XSS Attack!\");</script>"
		    "<p>A warning is printed in the logs.</p>");

SAMPLE_END(return std::move(text))
