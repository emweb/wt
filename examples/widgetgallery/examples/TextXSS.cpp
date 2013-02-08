#include <Wt/WText>

SAMPLE_BEGIN(TextXSS)
Wt::WText *text 
    = new Wt::WText("<p>This XHTML text contains JavaScript, "
		    "wich is filtered by the XSS filter.</p>"
		    "<script>alert(\"XSS Attack!\");</script>"
		    "<p>A warning is printed in the logs.</p>");

SAMPLE_END(return text)
