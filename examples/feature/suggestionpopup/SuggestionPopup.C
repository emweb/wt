/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WApplication.h>
#include <Wt/WContainerWidget.h>
#include <Wt/WPushButton.h>
#include <Wt/WSuggestionPopup.h>
#include <Wt/WLineEdit.h>
#include <Wt/WStringListModel.h>
#include <Wt/WText.h>
#include <Wt/WBreak.h>

using namespace Wt;

/*
 * See also: http://www.webtoolkit.eu/wt/blog/2010/03/02/javascript_that_is_c__
 */
#define INLINE_JAVASCRIPT(...) #__VA_ARGS__

class SuggestionPopups : public WApplication
{
public:
  SuggestionPopups(const WEnvironment& env)
    : WApplication(env)
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
  std::shared_ptr<WStringListModel> fourCharModel_;

  std::shared_ptr<WAbstractItemModel> createSimpleDrugsModel()
  {
    std::vector<std::string> hivDrugs{
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
    };

    std::shared_ptr<WStringListModel> model
        = std::make_shared<WStringListModel>();

    for (auto &drug : hivDrugs) {
      model->addString(drug);
    }

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
  std::shared_ptr<WAbstractItemModel> createDrugsModel()
  {
    std::vector<std::string> hivDrugs{
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
    };

    std::shared_ptr<WStringListModel> model
        = std::make_shared<WStringListModel>();

    for (auto &drug : hivDrugs) {
      int row = model->rowCount();

      model->addString(drug);

      std::string value = drug;
      std::size_t sc = value.find(';');
      if (sc != std::string::npos)
	value = value.substr(0, sc);
      model->setData(row, 0, value, ItemDataRole::User);
    }

    model->sort(0);

    return model;
  }

  void mostSimplePopup(WContainerWidget *parent)
  {
    WSuggestionPopup::Options simpleOptions;

    simpleOptions.highlightBeginTag = "<b>";
    simpleOptions.highlightEndTag = "</b>";
    simpleOptions.listSeparator = 0;

    std::unique_ptr<WSuggestionPopup> popup = cpp14::make_unique<WSuggestionPopup>(simpleOptions);
    popup->setModel(createSimpleDrugsModel());

    parent->addWidget(cpp14::make_unique<WText>(WString::tr("simplest-popup")));

    WLineEdit *edit = parent->addWidget(cpp14::make_unique<WLineEdit>());
    edit->resize(150, WLength::Auto);
    popup->forEdit(edit);

    parent->addChild(std::move(popup));
  }

  void simplePopup(WContainerWidget *parent)
  {
    WSuggestionPopup *popup = createAliasesMatchingPopup(parent);
    popup->setModel(createDrugsModel());
    popup->setMinimumSize(150, WLength::Auto);
    popup->setMaximumSize(WLength::Auto, 300);

    parent->addWidget(cpp14::make_unique<WText>(WString::tr("simple-popup-editing")));

    WLineEdit *edit = parent->addWidget(cpp14::make_unique<WLineEdit>());
    edit->resize(150, WLength::Auto);
    popup->forEdit(edit);

    parent->addWidget(cpp14::make_unique<WText>(WString::tr("simple-popup-dropdown")));

    edit = parent->addWidget(cpp14::make_unique<WLineEdit>());
    edit->resize(150, WLength::Auto);
    popup->forEdit(edit, PopupTrigger::DropDownIcon);

    /*
      showAt() shows the suggestion popup

      WPushButton *show = parent->addWidget(cpp14::make_unique<WPushButton>("show"));
      show->clicked().connect(std::bind(&WSuggestionPopup::showAt,
					  popup, edit));
    */
  }

  void serverSideFilteringPopups(WContainerWidget *parent)
  {
    fourCharModel_ = std::make_shared<WStringListModel>();

    WSuggestionPopup *popup = createAliasesMatchingPopup(parent);
    popup->setModel(fourCharModel_);
    popup->setFilterLength(3);
    popup->filterModel().connect(this, &SuggestionPopups::filter);
    popup->setMinimumSize(150, WLength::Auto);
    popup->setMaximumSize(WLength::Auto, 300);

    parent->addWidget(cpp14::make_unique<WText>(WString::tr("serverside-popup-editing")));

    WLineEdit *edit = parent->addWidget(cpp14::make_unique<WLineEdit>());

    edit->resize(150, WLength::Auto);
    popup->forEdit(edit, PopupTrigger::Editing);

    parent->addWidget(cpp14::make_unique<WText>(WString::tr("serverside-popup-dropdown")));

    edit = parent->addWidget(cpp14::make_unique<WLineEdit>());
    edit->resize(150, WLength::Auto);
    popup->forEdit(edit, PopupTrigger::DropDownIcon);
  }

  void filter(const WString& input)
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
	fourCharModel_->setData(row, 0, std::string(""), ItemDataRole::User);
	fourCharModel_->setData(row, 0, std::string("Wt-more-data"),
				ItemDataRole::StyleClass);

	break;
      }

      std::u32string v = input;
      while (v.length() < 3)
        v += U"a";

      v += (U"a"[0] + i);
      
      fourCharModel_->addString(v);
    }
  }

  WSuggestionPopup *createAliasesMatchingPopup(WContainerWidget *parent)
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

    return parent->addChild(
	cpp14::make_unique<WSuggestionPopup>(matcherJS, replacerJS));
  }
};

std::unique_ptr<WApplication> createApplication(const WEnvironment& env)
{
  return cpp14::make_unique<SuggestionPopups>(env);
}

int main(int argc, char **argv)
{
  return WRun(argc, argv, &createApplication);
}
