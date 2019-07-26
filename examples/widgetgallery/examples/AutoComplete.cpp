#include <Wt/WContainerWidget.h>
#include <Wt/WLineEdit.h>
#include <Wt/WSuggestionPopup.h>

SAMPLE_BEGIN(AutoComplete)

auto container = Wt::cpp14::make_unique<Wt::WContainerWidget>();

// Set options for email address suggestions:
Wt::WSuggestionPopup::Options contactOptions;
contactOptions.highlightBeginTag = "<span class=\"highlight\">";
contactOptions.highlightEndTag = "</span>";
contactOptions.listSeparator = ',';
contactOptions.whitespace = " \\n";
contactOptions.wordSeparators = "-., \"@\\n;";
contactOptions.appendReplacedText = ", ";

Wt::WSuggestionPopup *sp =
    container->addNew<Wt::WSuggestionPopup>(
            Wt::WSuggestionPopup::generateMatcherJS(contactOptions),
            Wt::WSuggestionPopup::generateReplacerJS(contactOptions));

Wt::WLineEdit *le = container->addNew<Wt::WLineEdit>();
le->setPlaceholderText("Enter a name starting with 'J'");
sp->forEdit(le);

// Populate the underlying model with suggestions:
sp->addSuggestion("John Tech <techie@mycompany.com>");
sp->addSuggestion("Johnny Cash <cash@mycompany.com>");
sp->addSuggestion("John Rambo <rambo@mycompany.com>");
sp->addSuggestion("Johanna Tree <johanna@mycompany.com>");

SAMPLE_END(return std::move(container))
