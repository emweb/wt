/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WApplication>
#include <Wt/WContainerWidget>
#include <Wt/WPushButton>
#include <Wt/WSuggestionPopup>
#include <Wt/WLineEdit>
#include <Wt/WStringListModel>
#include <Wt/WText>

/*
 * See also: http://www.webtoolkit.eu/wt/blog/2010/03/02/javascript_that_is_c__
 */
#define INLINE_JAVASCRIPT(...) #__VA_ARGS__

class SuggestionPopups : public Wt::WApplication
{
public:
  SuggestionPopups(const Wt::WEnvironment& env)
    : Wt::WApplication(env)
  {
    setTitle("WSuggestionPopup example");
    setCssTheme("polished");
    messageResourceBundle().use(appRoot() + "text");

    styleSheet().addRule(".Wt-suggest b", "color: black;");

    mostSimplePopup(root());
    simplePopup(root());
    serverSideFilteringPopups(root());
  }

private:
  Wt::WStringListModel *fourCharModel_;

  Wt::WAbstractItemModel *createSimpleDrugsModel()
  {
    const char *hivDrugs[] = {
      "Delavirdine",
      "Efavirenz",
      "Etravirine",
      "Nevirapine",
      "Abacavir",
      "Didanosine",
      "Emtricitabine",
      "Lamivudine",
      "Stavudine",
      "Tenofovir DF",
      "Zidovudine",
      "Amprenavir",
      "Atazanavir",
      "Darunavir",
      "Fosamprenavir",
      "Indinavir",
      "Lopinavir, Ritonavir",
      "Nelfinavir",
      "Ritonavir",
      "Saquinavir",
      "Tipranavir",
      0
    };

    Wt::WStringListModel *model = new Wt::WStringListModel();

    for (const char **i = hivDrugs; *i; ++i)
      model->addString(Wt::WString::fromUTF8(*i));

    return model;
  }

  /*
   * Creates a one-column model with data on HIV drugs.
   *  - the DisplayRole contains a semi-colon separated list of all names,
   *    starting with the leading name
   *  - the UserRole contains the leading name
   *
   * The DisplayRole will be interpreted by the special aliasing match
   * function
   */
  Wt::WAbstractItemModel *createDrugsModel()
  {
    const char *hivDrugs[] = {
      "Delavirdine;Rescriptor;DLV",
      "Efavirenz;Sustiva;EFV",
      "Etravirine;Intelence;TMC125;ETR",
      "Nevirapine;Viramune;NVP",
      "Abacavir;Ziagen;ABC",
      "Didanosine;Videx;ddI;Videx EC",
      "Emtricitabine;Emtriva;FTC",
      "Lamivudine;Epivir;3TC",
      "Stavudine;Zerit;d4T",
      "Tenofovir DF;Viread,TDF",
      "Zidovudine;Retrovir;AZT;ZDV",
      "Amprenavir;Agenerase;APV",
      "Atazanavir;Reyataz;ATV",
      "Darunavir;Prezista;TMC114;DRV",
      "Fosamprenavir;Lexiva;FPV",
      "Indinavir;Crixivan;IDV",
      "Lopinavir, Ritonavir;Kaletra;LPV/r",
      "Nelfinavir;Viracept;NFV",
      "Ritonavir;Norvir;RTV",
      "Saquinavir;Invirase;SQV",
      "Tipranavir;Aptivus;TPV",
      0
    };

    Wt::WStringListModel *model = new Wt::WStringListModel(this);

    for (const char **i = hivDrugs; *i; ++i) {
      int row = model->rowCount();

      std::string names = *i;
      model->addString(names);

      std::string value = names;
      std::size_t sc = value.find(';');
      if (sc != std::string::npos)
	value = value.substr(0, sc);
      model->setData(row, 0, value, Wt::UserRole);
    }

    model->sort(0);

    return model;
  }

  void mostSimplePopup(Wt::WContainerWidget *parent)
  {
    Wt::WSuggestionPopup::Options simpleOptions;
    simpleOptions.highlightBeginTag = "<b>";
    simpleOptions.highlightEndTag = "</b>";
    simpleOptions.listSeparator = 0;

    Wt::WSuggestionPopup *popup = new Wt::WSuggestionPopup(simpleOptions,
							   parent);
    popup->setModel(createSimpleDrugsModel());

    new Wt::WText(Wt::WString::tr("simplest-popup"), parent);

    Wt::WLineEdit *edit = new Wt::WLineEdit(parent);
    edit->resize(150, Wt::WLength::Auto);
    popup->forEdit(edit);
  }

  void simplePopup(Wt::WContainerWidget *parent)
  {
    Wt::WSuggestionPopup *popup = createAliasesMatchingPopup(parent);
    popup->setModel(createDrugsModel());
    popup->setMinimumSize(150, Wt::WLength::Auto);
    popup->setMaximumSize(Wt::WLength::Auto, 300);

    new Wt::WText(Wt::WString::tr("simple-popup-editing"), parent);

    Wt::WLineEdit *edit = new Wt::WLineEdit(parent);
    edit->resize(150, Wt::WLength::Auto);
    popup->forEdit(edit);

    new Wt::WText(Wt::WString::tr("simple-popup-dropdown"), parent);

    edit = new Wt::WLineEdit(parent);
    edit->resize(150, Wt::WLength::Auto);
    popup->forEdit(edit, Wt::WSuggestionPopup::DropDownIcon);

    /*
      showAt() shows the suggestion popup

      Wt::WPushButton *show = new Wt::WPushButton("show", parent);
      show->clicked().connect(boost::bind(&Wt::WSuggestionPopup::showAt,
					  popup, edit));
    */
  }

  void serverSideFilteringPopups(Wt::WContainerWidget *parent)
  {
    fourCharModel_ = new Wt::WStringListModel(this);

    Wt::WSuggestionPopup *popup = createAliasesMatchingPopup(parent);
    popup->setModel(fourCharModel_);
    popup->setFilterLength(3);
    popup->filterModel().connect(this, &SuggestionPopups::filter);
    popup->setMinimumSize(150, Wt::WLength::Auto);
    popup->setMaximumSize(Wt::WLength::Auto, 300);

    new Wt::WText(Wt::WString::tr("serverside-popup-editing"), parent);

    Wt::WLineEdit *edit = new Wt::WLineEdit(parent);

    edit->resize(150, Wt::WLength::Auto);
    popup->forEdit(edit, Wt::WSuggestionPopup::Editing);

    new Wt::WText(Wt::WString::tr("serverside-popup-dropdown"), parent);

    edit = new Wt::WLineEdit(parent);
    edit->resize(150, Wt::WLength::Auto);
    popup->forEdit(edit, Wt::WSuggestionPopup::DropDownIcon);
  }

  void filter(const Wt::WString& input)
  {
    /*
     * We implement a virtual model contains all items that start with
     * any arbitrary 3 characters, followed by "a-z"
     */
    fourCharModel_->removeRows(0, fourCharModel_->rowCount());

    for (int i = 0; i < 26; ++i) {
      int row = fourCharModel_->rowCount();

      /*
       * If the input is shorter than the server-side filter length,
       * then limit the number of matches and end with a '...'
       */
      if (input.value().length() < 3 && i > 10) {
	fourCharModel_->addString("...");
	fourCharModel_->setData(row, 0, std::string(""), Wt::UserRole);
	fourCharModel_->setData(row, 0, std::string("Wt-more-data"),
				Wt::StyleClassRole);

	break;
      }

      std::wstring v = input;
      while (v.length() < 3)
	v += L'a';

      v += (L'a' + i);
      
      fourCharModel_->addString(v);
    }
  }

  Wt::WSuggestionPopup *createAliasesMatchingPopup(Wt::WContainerWidget *parent)
  {
    /*
     * This matcher JavaScript function matches the input against the
     * name of a product, or one or more aliases.
     *
     * A match is indicated by product name and optionally matching aliases
     * between brackets.
     */

    /*
     * Note!
     *
     * INLINE_JAVASCRIPT is a macro which allows entry of JavaScript
     * directly in a C++ file.
     */
    std::string matcherJS = INLINE_JAVASCRIPT
      (
       function (edit) {
	 var value = edit.value;

	 return function(suggestion) {
	   if (!suggestion)
	     return value;
	   
	   var i, il,
	     names = suggestion.split(';'),
	     val = value.toUpperCase(),
	     matchedAliases = [],
	     matched = null;

	   if (val.length) {
	     for (i = 0, il = names.length; i < il; ++i) {
	       var name = names[i];
	       if (name.length >= val.length
		   && name.toUpperCase().substr(0, val.length) == val) {
		 // This name matches
		 name = '<b>' + name.substr(0, val.length) + '</b>'
		   + name.substr(val.length);

		 if (i == 0) // it's the product name
		   matched = name;
		 else // it's an alias
		   matchedAliases.push(name);
	       }
	     }
	   }

	   // Let '...' always match
	   if (names[0] == '...')
	     matched = names[0];

	   if (matched || matchedAliases.length) {
	     if (!matched)
	       matched = names[0];

	     if (matchedAliases.length)
	       matched += " (" + matchedAliases.join(", ") + ")";

	     return { match : true,
		      suggestion : matched };
	   } else {
	     return { match : false,
		      suggestion : names[0] };
	   }
	 }
       }
       );

    std::string replacerJS = INLINE_JAVASCRIPT
      (
       function (edit, suggestionText, suggestionValue) {
	 edit.value = suggestionValue;

	 if (edit.selectionStart)
	   edit.selectionStart = edit.selectionEnd = suggestionValue.length;
       }
       );

    return new Wt::WSuggestionPopup(matcherJS, replacerJS, parent);
  }
};

Wt::WApplication *createApplication(const Wt::WEnvironment& env)
{
  return new SuggestionPopups(env);
}

int main(int argc, char **argv)
{
  return Wt::WRun(argc, argv, &createApplication);
}
