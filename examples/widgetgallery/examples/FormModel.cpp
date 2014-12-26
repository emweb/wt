#include <Wt/WApplication>
#include <Wt/WBoostAny>
#include <Wt/WComboBox>
#include <Wt/WDate>
#include <Wt/WDateEdit>
#include <Wt/WDateValidator>
#include <Wt/WFormModel>
#include <Wt/WImage>
#include <Wt/WIntValidator>
#include <Wt/WLengthValidator>
#include <Wt/WModelIndex>
#include <Wt/WPushButton>
#include <Wt/WSpinBox>
#include <Wt/WStandardItem>
#include <Wt/WStandardItemModel>
#include <Wt/WString>
#include <Wt/WTemplateFormView>
#include <Wt/WTextArea>
#include <Wt/WValidator>

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

    UserFormModel(Wt::WObject *parent = 0)
        : Wt::WFormModel(parent)
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

    Wt::WAbstractItemModel *countryModel() {
        return countryModel_;
    }

    int countryModelRow(const std::string& code) {
	for (int i = 0; i < countryModel_->rowCount(); ++i)
	    if (countryCode(i) == code)
	        return i;

	return -1;
    }

    Wt::WAbstractItemModel *cityModel() {
        return cityModel_;
    }

    void updateCityModel(const std::string& countryCode) {
        cityModel_->clear();

        CityMap::const_iterator i = cities.find(countryCode);

        if (i != cities.end()) {
            const std::vector<std::string>& cities = i->second;

            // The initial text shown in the city combo box should be an empty
            // string.
            cityModel_->appendRow(new Wt::WStandardItem());

            for (unsigned j = 0; j < cities.size(); ++j)
                cityModel_->appendRow(new Wt::WStandardItem(cities[j]));
        } else {
            cityModel_->appendRow(
                        new Wt::WStandardItem("(Choose Country first)"));
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
        return boost::any_cast<std::string>
	  (countryModel_->data(row, 0, Wt::UserRole));
    }

    typedef std::map< std::string, std::vector<std::string> > CityMap;
    typedef std::map<std::string, std::string> CountryMap;

private:
    static const CityMap cities;
    static const CountryMap countries;
    Wt::WStandardItemModel *countryModel_, *cityModel_;

    static const int MAX_LENGTH = 25;
    static const int MAX_CHILDREN = 15;

    void initializeModels() {
        // Create a country model.
        unsigned countryModelRows = countries.size() + 1;
        const unsigned countryModelColumns = 1;
        countryModel_ =
          new Wt::WStandardItemModel(countryModelRows, countryModelColumns,
                                     this);

        // The initial text shown in the country combo box should be an empty
        // string.
        int row = 0;
        countryModel_->setData(row, 0, std::string(" "), Wt::DisplayRole);
        countryModel_->setData(row, 0, std::string(), Wt::UserRole);

        // For each country, update the model based on the key (corresponding
        // to the country code):
        // - set the country name for the display role,
        // - set the city names for the user role.
        row = 1;
        for (CountryMap::const_iterator i = countries.begin();
                                        i != countries.end(); ++i) {
            countryModel_->setData(row, 0, i->second, Wt::DisplayRole);
            countryModel_->setData(row++, 0, i->first, Wt::UserRole);
        }

        // Create a city model.
        cityModel_ = new Wt::WStandardItemModel(this);
        updateCityModel(std::string());
    }

    Wt::WValidator *createNameValidator(const std::string& field) {
        Wt::WLengthValidator *v = new Wt::WLengthValidator();
        v->setMandatory(true);
        v->setMinimumLength(1);
        v->setMaximumLength(MAX_LENGTH);
        return v;
    }

    Wt::WValidator *createCountryValidator() {
        Wt::WLengthValidator *v = new Wt::WLengthValidator();
        v->setMandatory(true);
        return v;
    }

    Wt::WValidator *createCityValidator() {
        Wt::WLengthValidator *v = new Wt::WLengthValidator();
        v->setMandatory(true);
        return v;
    }

    Wt::WValidator *createBirthValidator() {
        Wt::WDateValidator *v = new Wt::WDateValidator();
        v->setBottom(Wt::WDate(1900, 1, 1));
        v->setTop(Wt::WDate::currentDate());
        v->setFormat("dd/MM/yyyy");
        v->setMandatory(true);
        return v;
    }

    Wt::WValidator *createChildrenValidator() {
        Wt::WIntValidator *v = new Wt::WIntValidator(0, MAX_CHILDREN);
        v->setMandatory(true);
        return v;
    }

};

const Wt::WFormModel::Field UserFormModel::FirstNameField = "first-name";
const Wt::WFormModel::Field UserFormModel::LastNameField = "last-name";
const Wt::WFormModel::Field UserFormModel::CountryField = "country";
const Wt::WFormModel::Field UserFormModel::CityField = "city";
const Wt::WFormModel::Field UserFormModel::BirthField = "birth";
const Wt::WFormModel::Field UserFormModel::ChildrenField = "children";
const Wt::WFormModel::Field UserFormModel::RemarksField = "remarks";

// In C++11, this initializing can be done inline, within the declaration:
//
// const UserFormModel::CityMap UserFormModel::cities = {
//    { "BE", { "Antwerp", "Bruges", "Brussels", "Ghent" } },
//    { "NL", { "Amsterdam", "Eindhoven", "Rotterdam", "The Hague"} },
//    { "UK", { "London", "Bristol", "Oxford", "Stonehenge"} },
//    { "US", { "Boston", "Chicago", "Los Angeles", "New York"} }
// };

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

class UserFormView : public Wt::WTemplateFormView
{
public:
    // inline constructor
    UserFormView() {
        model = new UserFormModel(this);

        setTemplateText(tr("userForm-template"));
        addFunction("id", &WTemplate::Functions::id);
        addFunction("block", &WTemplate::Functions::id);

        /*
	 * First Name
	 */
	setFormWidget(UserFormModel::FirstNameField, new Wt::WLineEdit());

	/*
	 * Last Name
	 */
	setFormWidget(UserFormModel::LastNameField, new Wt::WLineEdit());

	/*
	 * Country
	 */
	Wt::WComboBox *countryCB = new Wt::WComboBox();
	countryCB->setModel(model->countryModel());

	countryCB->activated().connect(std::bind([=] () {
	    std::string code = model->countryCode(countryCB->currentIndex());
	    model->updateCityModel(code);
	}));

	setFormWidget(UserFormModel::CountryField, countryCB,
            [=] () { // updateViewValue()
	        std::string code = boost::any_cast<std::string>
		    (model->value(UserFormModel::CountryField));
		int row = model->countryModelRow(code);
		countryCB->setCurrentIndex(row);
	    },

            [=] () { // updateModelValue()
	        std::string code = model->countryCode(countryCB->currentIndex());
		model->setValue(UserFormModel::CountryField, code);
            });

	/*
	 * City
	 */
	Wt::WComboBox *cityCB = new Wt::WComboBox();
	cityCB->setModel(model->cityModel());
	setFormWidget(UserFormModel::CityField, cityCB);

	/*
	 * Birth Date
	 */
	Wt::WDateEdit *dateEdit = new Wt::WDateEdit();
	setFormWidget(UserFormModel::BirthField, dateEdit,
	    [=] () { // updateViewValue()
	        Wt::WDate date = boost::any_cast<Wt::WDate>
		    (model->value(UserFormModel::BirthField));
                dateEdit->setDate(date);
	    }, 

            [=] () { // updateModelValue()
	        Wt::WDate date = dateEdit->date();
                model->setValue(UserFormModel::BirthField, date);
	    });

        /*
	 * Children
	 */ 
	setFormWidget(UserFormModel::ChildrenField, new Wt::WSpinBox());

	/*
	 * Remarks
	 */
	Wt::WTextArea *remarksTA = new Wt::WTextArea();
	remarksTA->setColumns(40);
	remarksTA->setRows(5);
	setFormWidget(UserFormModel::RemarksField, remarksTA);

	/*
	 * Title & Buttons
	 */
        Wt::WString title = Wt::WString("Create new user");
        bindString("title", title);

        Wt::WPushButton *button = new Wt::WPushButton("Save");
        bindWidget("submit-button", button);

        bindString("submit-info", Wt::WString());

        button->clicked().connect(this, &UserFormView::process);

        updateView(model);
    }

private:
    void process() {
        updateModel(model);

        if (model->validate()) {
            // Do something with the data in the model: show it.
            bindString("submit-info",
                       Wt::WString::fromUTF8("Saved user data for ")
		       + model->userData(), Wt::PlainText);
            // Udate the view: Delete any validation message in the view, etc.
            updateView(model);
            // Set the focus on the first field in the form.
            Wt::WLineEdit *viewField =
                    resolve<Wt::WLineEdit*>(UserFormModel::FirstNameField);
            viewField->setFocus(true);
        } else {
            bindEmpty("submit-info"); // Delete the previous user data.
            updateView(model);
        }
    }

    UserFormModel *model;
};

SAMPLE_BEGIN(FormModel)

UserFormView *view = new UserFormView();

SAMPLE_END(return view)
