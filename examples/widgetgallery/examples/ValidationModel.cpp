#include <Wt/WFormModel.h>
#include <Wt/WIntValidator.h>
#include <Wt/WLineEdit.h>
#include <Wt/WPushButton.h>
#include <Wt/WValidator.h>
#include <Wt/WTemplateFormView.h>

class AgeFormModel : public Wt::WFormModel
{
public:
    static const Field AgeField;

    // inline constructor
    AgeFormModel() : WFormModel()
    {
        addField(AgeField);
        setValidator(AgeField, createAgeValidator());
        setValue(AgeField, std::string());
    }

private:
    std::shared_ptr<Wt::WValidator> createAgeValidator() {
        return std::make_shared<Wt::WIntValidator>(0, 150);
    }
};

const Wt::WFormModel::Field AgeFormModel::AgeField = "age";

class AgeFormView : public Wt::WTemplateFormView
{
public:
    // inline constructor
    AgeFormView() {
        model_ = std::make_unique<AgeFormModel>();

        setTemplateText(tr("validation-template"));

        setFormWidget(AgeFormModel::AgeField, std::make_unique<Wt::WLineEdit>());

        auto button = bindWidget("button",
                                 std::make_unique<Wt::WPushButton>("Save"));

        button->clicked().connect(this, &AgeFormView::process);

        updateView(model_.get());
    }

private:
    void process() {
        updateModel(model_.get());
        if (model_->validate()) {
            // Udate the view: Delete any validation message in the view, etc.
            updateView(model_.get());
            bindString("age-info",
                       Wt::WString("Age of {1} is saved!")
                       .arg(Wt::asString(model_->value(AgeFormModel::AgeField))));
        } else {
            updateView(model_.get());
            // Set the focus on the line edit.
            Wt::WLineEdit *viewField =
                       resolve<Wt::WLineEdit *>(AgeFormModel::AgeField);
            viewField->setFocus(true);
        }
    }

    std::unique_ptr<AgeFormModel> model_;
};

SAMPLE_BEGIN(ValidationModel)

auto view = std::make_unique<AgeFormView>();

SAMPLE_END(return std::move(view))
