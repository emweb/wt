#include <Wt/WApplication.h>
#include <Wt/WAny.h>
#include <Wt/WComboBox.h>
#include <Wt/WDate.h>
#include <Wt/WDateEdit.h>
#include <Wt/WDateValidator.h>
#include <Wt/WFormModel.h>
#include <Wt/WImage.h>
#include <Wt/WIntValidator.h>
#include <Wt/WLengthValidator.h>
#include <Wt/WModelIndex.h>
#include <Wt/WPushButton.h>
#include <Wt/WSpinBox.h>
#include <Wt/WStandardItem.h>
#include <Wt/WStandardItemModel.h>
#include <Wt/WString.h>
#include <Wt/WTemplateFormView.h>
#include <Wt/WTextArea.h>
#include <Wt/WValidator.h>

class UserFormModel : public Wt::WFormModel
{
public:
    // Associate each field with a unique string literal.
    static const Field FirstNameField;
    static const Field LastNameField;
    static const Field CountryField;
    static const Field CityField;
    static const Field BirthField;
    static const Field ChildrenField;
    static const Field RemarksField;

    UserFormModel()
        : WFormModel()
    {
        initializeModels();

        addField(FirstNameField);
        addField(LastNameField);
        addField(CountryField);
        addField(CityField);
        addField(BirthField);
        addField(ChildrenField);
        addField(RemarksField);

        setValidator(FirstNameField, createNameValidator(FirstNameField));
        setValidator(LastNameField, createNameValidator(LastNameField));
        setValidator(CountryField, createCountryValidator());
        setValidator(CityField, createCityValidator());
        setValidator(BirthField, createBirthValidator());
        setValidator(ChildrenField, createChildrenValidator());

        // Here you could populate the model with initial data using
        // setValue() for each field.
        setValue(BirthField, Wt::WDate());
        setValue(CountryField, std::string());
    }

    std::shared_ptr<Wt::WAbstractItemModel> countryModel() {
        return countryModel_;
    }

    int countryModelRow(const std::string& code) {
	for (int i = 0; i < countryModel_->rowCount(); ++i)
	    if (countryCode(i) == code)
	        return i;

	return -1;
    }

    std::shared_ptr<Wt::WAbstractItemModel> cityModel() {
        return cityModel_;
    }

    void updateCityModel(const std::string& countryCode) {
        cityModel_->clear();

        CityMap::const_iterator i = cities.find(countryCode);

        if (i != cities.end()) {
            const std::vector<std::string>& cities = i->second;

            // The initial text shown in the city combo box should be an empty
            // string.
            cityModel_->appendRow(std::make_unique<Wt::WStandardItem>());

            for (unsigned j = 0; j < cities.size(); ++j)
                cityModel_->appendRow(std::make_unique<Wt::WStandardItem>(cities[j]));
        } else {
            cityModel_->appendRow(
                        std::make_unique<Wt::WStandardItem>("(Choose Country first)"));
        }
    }

    // Get the user data from the model
    Wt::WString userData() {
        return
        Wt::asString(value(FirstNameField)) + " " +
        Wt::asString(value(LastNameField))
        + ": country code=" + Wt::asString(value(CountryField))
        + ", city=" + Wt::asString(value(CityField))
        + ", birth=" + Wt::asString(value(BirthField))
        + ", children=" + Wt::asString(value(ChildrenField))
        + ", remarks=" + Wt::asString(value(RemarksField))
        + ".";
    }

    // Get the right code for the current index.
    std::string countryCode (int row) {
        return Wt::asString(countryModel_->data(row, 0, Wt::ItemDataRole::User)).toUTF8();
    }

    typedef std::map< std::string, std::vector<std::string> > CityMap;
    typedef std::map<std::string, std::string> CountryMap;

private:
    static const CityMap cities;
    static const CountryMap countries;
    std::shared_ptr<Wt::WStandardItemModel> countryModel_, cityModel_;

    static const int MAX_LENGTH;
    static const int MAX_CHILDREN;

    void initializeModels() {
        // Create a country model.
        unsigned countryModelRows = countries.size() + 1;
        const unsigned countryModelColumns = 1;
        countryModel_ =
          std::make_shared<Wt::WStandardItemModel>(countryModelRows, countryModelColumns);

        // The initial text shown in the country combo box should be an empty
        // string.
        int row = 0;
        countryModel_->setData(row, 0, std::string(" "), Wt::ItemDataRole::Display);
        countryModel_->setData(row, 0, std::string(), Wt::ItemDataRole::User);

        // For each country, update the model based on the key (corresponding
        // to the country code):
        // - set the country name for the display role,
        // - set the city names for the user role.
        row = 1;
        for (CountryMap::const_iterator i = countries.begin();
                                        i != countries.end(); ++i) {
            countryModel_->setData(row, 0, i->second, Wt::ItemDataRole::Display);
            countryModel_->setData(row++, 0, i->first, Wt::ItemDataRole::User);
        }

        // Create a city model.
        cityModel_ = std::make_shared<Wt::WStandardItemModel>();
        updateCityModel(std::string());
    }

    std::shared_ptr<Wt::WValidator> createNameValidator(const std::string& field) {
        auto v = std::make_shared<Wt::WLengthValidator>();
        v->setMandatory(true);
        v->setMinimumLength(1);
        v->setMaximumLength(MAX_LENGTH);
        return v;
    }

    std::shared_ptr<Wt::WValidator> createCountryValidator() {
        auto v = std::make_shared<Wt::WLengthValidator>();
        v->setMandatory(true);
        return v;
    }

    std::shared_ptr<Wt::WValidator> createCityValidator() {
        auto v = std::make_shared<Wt::WLengthValidator>();
        v->setMandatory(true);
        return v;
    }

    std::shared_ptr<Wt::WValidator> createBirthValidator() {
        auto v = std::make_shared<Wt::WDateValidator>();
        v->setBottom(Wt::WDate(1900, 1, 1));
        v->setTop(Wt::WDate::currentDate());
        v->setFormat("dd/MM/yyyy");
        v->setMandatory(true);
        return v;
    }

    std::shared_ptr<Wt::WValidator> createChildrenValidator() {
        auto v = std::make_shared<Wt::WIntValidator>(0, MAX_CHILDREN);
        v->setMandatory(true);
        return v;
    }

};

const int UserFormModel::MAX_LENGTH = 25;
const int UserFormModel::MAX_CHILDREN = 15;

const Wt::WFormModel::Field UserFormModel::FirstNameField = "first-name";
const Wt::WFormModel::Field UserFormModel::LastNameField = "last-name";
const Wt::WFormModel::Field UserFormModel::CountryField = "country";
const Wt::WFormModel::Field UserFormModel::CityField = "city";
const Wt::WFormModel::Field UserFormModel::BirthField = "birth";
const Wt::WFormModel::Field UserFormModel::ChildrenField = "children";
const Wt::WFormModel::Field UserFormModel::RemarksField = "remarks";

#ifndef WT_TARGET_JAVA
const UserFormModel::CountryMap UserFormModel::countries = {
  { "BE", { "Belgium" } },
  { "NL", { "Netherlands" } },
  { "UK", { "United Kingdom" } },
  { "US", { "United States" } }
};
#else // WT_TARGET_JAVA
namespace {
  UserFormModel::CountryMap getCountryMap() {
    UserFormModel::CountryMap retval;
    retval["BE"] = "Belgium";
    retval["NL"] = "Netherlands";
    retval["UK"] = "United Kingdom";
    retval["US"] = "United States";
    return retval;
  }
}
const UserFormModel::CountryMap UserFormModel::countries = getCountryMap();
#endif // WT_TARGET_JAVA

#ifndef WT_TARGET_JAVA
const UserFormModel::CityMap UserFormModel::cities = {
  { "BE", { "Antwerp", "Bruges", "Brussels", "Ghent" } },
  { "NL", { "Amsterdam", "Eindhoven", "Rotterdam", "The Hague"} },
  { "UK", { "London", "Bristol", "Oxford", "Stonehenge"} },
  { "US", { "Boston", "Chicago", "Los Angeles", "New York"} }
};
#else // WT_TARGET_JAVA
namespace {
  UserFormModel::CityMap getCityMap() {
    std::vector<std::string> beCities;
    beCities.push_back("Antwerp");
    beCities.push_back("Bruges");
    beCities.push_back("Brussels");
    beCities.push_back("Ghent");
    
    std::vector<std::string> nlCities;
    nlCities.push_back("Amsterdam");
    nlCities.push_back("Eindhoven");
    nlCities.push_back("Rotterdam");
    nlCities.push_back("The Hague");
    
    std::vector<std::string> ukCities;
    ukCities.push_back("London");
    ukCities.push_back("Bristol");
    ukCities.push_back("Oxford");
    ukCities.push_back("Stonehenge");
    
    std::vector<std::string> usCities;
    usCities.push_back("Boston");
    usCities.push_back("Chicago");
    usCities.push_back("Los Angeles");
    usCities.push_back("New York");
    
    UserFormModel::CityMap retval;
    retval["BE"] = beCities;
    retval["NL"] = nlCities;
    retval["UK"] = ukCities;
    retval["US"] = usCities;
    return retval;
  }
}
const UserFormModel::CityMap UserFormModel::cities = getCityMap();
#endif // WT_TARGET_JAVA

class UserFormView : public Wt::WTemplateFormView
{
public:
    // inline constructor
    UserFormView() {
        model = std::make_shared<UserFormModel>();

        setTemplateText(tr("userForm-template"));
        addFunction("id", &WTemplate::Functions::id);
        addFunction("block", &WTemplate::Functions::id);

        /*
	 * First Name
	 */
	setFormWidget(UserFormModel::FirstNameField,
	              std::make_unique<Wt::WLineEdit>());

	/*
	 * Last Name
	 */
	setFormWidget(UserFormModel::LastNameField,
	              std::make_unique<Wt::WLineEdit>());

	/*
	 * Country
	 */
	auto countryCB = std::make_unique<Wt::WComboBox>();
	auto countryCB_ = countryCB.get();
	countryCB->setModel(model->countryModel());

	countryCB_->activated().connect([=] {
	    std::string code = model->countryCode(countryCB_->currentIndex());
	    model->updateCityModel(code);
	});

        setFormWidget(UserFormModel::CountryField, std::move(countryCB),
            [=] { // updateViewValue()
                std::string code =
                    Wt::asString(model->value(UserFormModel::CountryField)).toUTF8();
		int row = model->countryModelRow(code);
		countryCB_->setCurrentIndex(row);
	    },

            [=] { // updateModelValue()
                std::string code = model->countryCode(countryCB_->currentIndex());
		model->setValue(UserFormModel::CountryField, code);
            });

	/*
	 * City
	 */
	auto cityCB = std::make_unique<Wt::WComboBox>();
	cityCB->setModel(model->cityModel());
	setFormWidget(UserFormModel::CityField, std::move(cityCB));

	/*
	 * Birth Date
	 */
	auto dateEdit = std::make_unique<Wt::WDateEdit>();
	auto dateEdit_ = dateEdit.get();
	setFormWidget(UserFormModel::BirthField, std::move(dateEdit),
	    [=] { // updateViewValue()
	        Wt::WDate date = Wt::cpp17::any_cast<Wt::WDate>
		    (model->value(UserFormModel::BirthField));
		dateEdit_->setDate(date);
	    }, 

            [=] { // updateModelValue()
                Wt::WDate date = dateEdit_->date();
                model->setValue(UserFormModel::BirthField, date);
	    });

        /*
	 * Children
	 */ 
	setFormWidget(UserFormModel::ChildrenField, std::make_unique<Wt::WSpinBox>());

	/*
	 * Remarks
	 */
	auto remarksTA = std::make_unique<Wt::WTextArea>();
	remarksTA->setColumns(40);
	remarksTA->setRows(5);
	setFormWidget(UserFormModel::RemarksField, std::move(remarksTA));

	/*
	 * Title & Buttons
	 */
	Wt::WString title = Wt::WString("Create new user");
        bindString("title", title);

        auto button = bindWidget("submit-button", std::make_unique<Wt::WPushButton>("Save"));

        bindString("submit-info", Wt::WString());

        button->clicked().connect(this, &UserFormView::process);

        updateView(model.get());
    }

private:
    void process() {
        updateModel(model.get());

        if (model->validate()) {
            // Do something with the data in the model: show it.
            bindString("submit-info",
                       Wt::WString("Saved user data for ")
                       + model->userData(), Wt::TextFormat::Plain);
            // Udate the view: Delete any validation message in the view, etc.
            updateView(model.get());
            // Set the focus on the first field in the form.
            Wt::WLineEdit *viewField =
                    resolve<Wt::WLineEdit*>(UserFormModel::FirstNameField);
            viewField->setFocus(true);
        } else {
            bindEmpty("submit-info"); // Delete the previous user data.
            updateView(model.get());
        }
    }

    std::shared_ptr<UserFormModel> model;
};

SAMPLE_BEGIN(FormModel)

auto view = std::make_unique<UserFormView>();

SAMPLE_END(return std::move(view))
