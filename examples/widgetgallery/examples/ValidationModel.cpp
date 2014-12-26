#include <Wt/WBoostAny>
#include <Wt/WFormModel>
#include <Wt/WIntValidator>
#include <Wt/WLineEdit>
#include <Wt/WPushButton>
#include <Wt/WValidator>
#include <Wt/WTemplateFormView>

class AgeFormModel : public Wt::WFormModel
{
public:
    // in C++11:
    // static constexpr Field AgeField = "age";
    static Field AgeField;

    // inline constructor
    AgeFormModel(Wt::WObject *parent = 0) : Wt::WFormModel(parent)
    {
        addField(AgeField);
        setValidator(AgeField, createAgeValidator());
        setValue(AgeField, std::string());
    }

private:
    Wt::WValidator *createAgeValidator() {
        Wt::WIntValidator *v = new Wt::WIntValidator(0, 150);
        return v;
    }
};

Wt::WFormModel::Field AgeFormModel::AgeField = "age";

class AgeFormView : public Wt::WTemplateFormView
{
public:
    // inline constructor
    AgeFormView() {
        model_ = new AgeFormModel(this);

        setTemplateText(tr("validation-template"));

	setFormWidget(AgeFormModel::AgeField, new Wt::WLineEdit());

        Wt::WPushButton *button = new Wt::WPushButton("Save");
        bindWidget("button", button);

        button->clicked().connect(this, &AgeFormView::process);

        updateView(model_);
    }

private:
    void process() {
        updateModel(model_);
        if (model_->validate()) {
            // Udate the view: Delete any validation message in the view, etc.
            updateView(model_);
            bindString("age-info",
                       Wt::WString("Age of {1} is saved!")
		       .arg(Wt::asString
			    (model_->value(AgeFormModel::AgeField))));
        } else {
            updateView(model_);
            // Set the focus on the line edit.
            Wt::WLineEdit *viewField =
                       resolve<Wt::WLineEdit*>(AgeFormModel::AgeField);
            viewField->setFocus(true);
        }
    }

    AgeFormModel *model_;
};

SAMPLE_BEGIN(ValidationModel)

AgeFormView *view = new AgeFormView();

SAMPLE_END(return view)
