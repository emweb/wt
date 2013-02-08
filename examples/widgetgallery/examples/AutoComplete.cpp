#include <Wt/WContainerWidget>
#include <Wt/WLineEdit>
#include <Wt/WSuggestionPopup>

SAMPLE_BEGIN(AutoComplete)
Wt::WContainerWidget *container = new Wt::WContainerWidget();

// Set options for email address suggestions:
Wt::WSuggestionPopup::Options contactOptions;
contactOptions.highlightBeginTag = "<span class=\"highlight\">";
contactOptions.highlightEndTag = "</span>";
contactOptions.listSeparator = ',';
contactOptions.whitespace = " \\n";
contactOptions.wordSeparators = "-., \"@\\n;";
contactOptions.appendReplacedText = ", ";

Wt::WSuggestionPopup *sp = new Wt::WSuggestionPopup(
	    Wt::WSuggestionPopup::generateMatcherJS(contactOptions),
	    Wt::WSuggestionPopup::generateReplacerJS(contactOptions),
	    container);

Wt::WLineEdit *le = new Wt::WLineEdit(container);
le->setEmptyText("Enter a name starting with 'J'");
sp->forEdit(le);

// Populate the underlying model with suggestions:
sp->addSuggestion("John Tech <techie@mycompany.com>");
sp->addSuggestion("Johnny Cash <cash@mycompany.com>");
sp->addSuggestion("John Rambo <rambo@mycompany.com>");
sp->addSuggestion("Johanna Tree <johanna@mycompany.com>");

SAMPLE_END(return container)
