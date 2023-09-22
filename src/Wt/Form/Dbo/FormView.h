// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2021 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_FORM_DBO_FORMVIEW_H_
#define WT_FORM_DBO_FORMVIEW_H_

#include <Wt/WTemplateFormView.h>
#include <Wt/Form/WAbstractFormDelegate.h>
#include <Wt/Form/Dbo/Actions.h>
#include <Wt/Form/Dbo/FormModel.h>

namespace Wt {
  namespace Form {
    namespace Dbo {
/*! \class FormView Wt/Form/Dbo/FormView.h Wt/Form/Dbo/FormView.h
 *  \brief A view class to represent database objects
 *
 * This class takes the database object as a template argument. Instead of having to manually
 * define all the widgets, the system will automatically generate default widgets based on the
 * data type used in the database. For example: a `WString` object will by default be represented
 * by a `WLineEdit` in the UI.
 *
 * \sa FormModel
 *
 * \ingroup form
 */
template<class C>
class FormView : public Wt::WTemplateFormView
{
public:
  /*! \brief Constructor
   *
   * \sa WTemplateFormView
   */
  explicit FormView(const Wt::WString& text)
    : Wt::WTemplateFormView(text)
  {
  }

  /*! \brief Sets the form model
   *
   * This method will automatically generate the form delegates
   * and set the form widgets and model validators.
   *
   * \sa customizeFormWidget, customizeValidator
   */
  void setFormModel(std::shared_ptr<FormModel<C>> model)
  {
    model_ = std::move(model);

    C dummy;

    // Automatically generate the form delegates
    ViewAction action(model_->session(), model_.get(), formDelegates_);
    dummy.persist(action);

    for (const Wt::WFormModel::Field& f : model_->fields()) {
      setFormWidget(f, formWidget(f));
      model_->setValidator(f, validator(f));
    }

    updateView(model_.get());
  }

  /*! \brief Sets a custom form delegate
   *
   * Overrides the default delegate for a given field
   *
   * \throws WException when this function is called after setFormModel
   */
  void setFormDelegate(Wt::WFormModel::Field field, std::shared_ptr<Wt::Form::WAbstractFormDelegate> delegate)
  {
    if (model_) {
      throw Wt::WException("Form Delegates cannot be set after the model has been initialized!");
    }

    if (!delegate) {
      // Erase from map
      auto it = formDelegates_.find(field);
      if (it != formDelegates_.end()) {
        formDelegates_.erase(it);
      }
    } else {
      formDelegates_[field] = std::move(delegate);
    }
  }

  /*! \brief Creates a form widget
   *
   * This method is to ensure that the correct widget is used in the form for the given
   * field, i.e. the widget defined by the form delegate.
   *
   * \sa WAbstractFormDelegate::createFormWidget
   */
  std::unique_ptr<Wt::WWidget> createFormWidget(Wt::WFormModel::Field field) override
  {
    return formWidget(field);
  }

  /*! \brief Updates a value in the Model
   *
   * The default implementation first calls updateModelValue(WFormModel *, WFormModel::Field, WWidget *).
   * If that returns false, it will then call the updateModelValue function of WAbstractFormDelegate if there's
   * a delegate defined for this field. If not, it will call the function of WTemplateFormView.
   */
  void updateModelValue(Wt::WFormModel *model, Wt::WFormModel::Field field, Wt::WFormWidget *edit) override
  {
    if (updateModelValue(model, field, static_cast<Wt::WWidget*>(edit))) {
      return;
    }

    std::shared_ptr<Wt::Form::WAbstractFormDelegate> d = delegate(field);
    if (d) {
      d->updateModelValue(model, field, edit);
    } else {
      Wt::WTemplateFormView::updateModelValue(model, field, edit);
    }
  }

  /*! \brief Updates a value in the Model
   *
   * The default implementation calls the updateModelValue function of WAbstractFormDelegate if there's a
   * delegate defined for this field. If not, it will call the function of WTemplateFormView.
   */
  bool updateModelValue(Wt::WFormModel *model, Wt::WFormModel::Field field, Wt::WWidget *edit) override
  {
    std::shared_ptr<Wt::Form::WAbstractFormDelegate> d = delegate(field);
    if (d) {
      return d->updateModelValue(model, field, edit);
    } else {
      return Wt::WTemplateFormView::updateModelValue(model, field, edit);
    }
  }

  /*! \brief Updates a value in the View
   *
   * The default implementation first calls updateViewValue(WFormModel *, WFormModel::Field, WWidget *).
   * If that returns false, it will then call the updateViewValue function of WAbstractFormDelegate if there's
   * a delegate defined for this field. If not, it will call the function of WTemplateFormView.
   */
  void updateViewValue(Wt::WFormModel *model, Wt::WFormModel::Field field, Wt::WFormWidget *edit) override
  {
    if (updateViewValue(model, field, static_cast<Wt::WWidget*>(edit))) {
      return;
    }

    std::shared_ptr<Wt::Form::WAbstractFormDelegate> d = delegate(field);
    if (d) {
      d->updateViewValue(model, field, edit);
    } else {
      Wt::WTemplateFormView::updateViewValue(model, field, edit);
    }
  }

  /*! \brief Updates a value in the View
   *
   * The default implementation calls the updateViewValue function of WAbstractFormDelegate if there's a
   * delegate defined for this field. If not, it will call the function of WTemplateFormView.
   */
  bool updateViewValue(Wt::WFormModel *model, Wt::WFormModel::Field field, Wt::WWidget *edit) override
  {
    std::shared_ptr<Wt::Form::WAbstractFormDelegate> d = delegate(field);
    if (d) {
      return d->updateViewValue(model, field, edit);
    } else {
      return Wt::WTemplateFormView::updateViewValue(model, field, edit);
    }
  }

  /*! \brief Saves the data to database
   *
   * First the data is validated. If the data in the model is valid, the data will be pushed to the database.
   * If not, the validationFailed() signal will be emitted. On succesful save, the saved() signal will be emitted.
   */
  void save()
  {
    updateModel(model_.get());
    if (model_->validate()) {
      model_->saveDboValues();
      updateView(model_.get());
      saved().emit();
    } else {
      // update the view to show the validation text
      updateView(model_.get());
      validationFailed().emit();
    }
  }

  /*! \brief Returns the form model used by this class
   */
  const std::shared_ptr<FormModel<C>>& model() const { return model_; }

protected:
  /*! \brief Customize the auto generate form widget
   *
   * Allows derived classes to customize the automatically generated widget
   * without having to customize an entire WFormDelegate.
   *
   * For example: the default widget for WString objects is a WLineEdit. This
   * method allows a derived class to set the input mask for a specific input field.
   *
   * Base class implementation doesn't modify the widget
   */
  virtual void customizeFormWidget(Wt::WFormModel::Field field, Wt::WWidget *widget)
  {
  }

  /*! \brief Customize the auto generated validator
   *
   * Allows derived classes to customize the automatically generated validator
   * without having to customize an entire WFormDelegate.
   *
   * For example: the default validator for integers is a WIntValidator. This
   * method allows a derived class to specify the range for the validator.
   *
   * Base class implementation doesn't modify the validator
   */
  virtual void customizeValidator(Wt::WFormModel::Field field, Wt::WValidator *validator)
  {
  }

  /*! \brief %Signal emitted when form is saved
   */
  Wt::Signal<>& saved() { return saved_; }

  /*! \brief %Signal emitted when validation failed
   *
   * This can be emitted when saving the form. The save action
   * will have failed because some fields are invalid.
   */
  Wt::Signal<>& validationFailed() { return validationFailed_; }

private:
  std::shared_ptr<FormModel<C>> model_;
  std::map<std::string, std::shared_ptr<Wt::Form::WAbstractFormDelegate>> formDelegates_;

  Wt::Signal<> saved_;
  Wt::Signal<> validationFailed_;

  /*! \brief Gets the widget generated by the form delegate
   */
  std::unique_ptr<Wt::WWidget> formWidget(Wt::WFormModel::Field field)
  {
    std::shared_ptr<Wt::Form::WAbstractFormDelegate> d = delegate(field);
    if (d) {
      std::unique_ptr<Wt::WWidget> widget = d->createFormWidget();
      customizeFormWidget(field, widget.get());
      return widget;
    }
    return nullptr;
  }

  /*! \brief Gets the validator generated by the form delegate
   */
  std::shared_ptr<Wt::WValidator> validator(Wt::WFormModel::Field field)
  {
    std::shared_ptr<Wt::Form::WAbstractFormDelegate> d = delegate(field);
    if (d) {
      std::shared_ptr<Wt::WValidator> validator = d->createValidator();
      customizeValidator(field, validator.get());
      return validator;
    }
    return nullptr;
  }

  /*! \brief Gets the form delegate
   */
  std::shared_ptr<Wt::Form::WAbstractFormDelegate> delegate(Wt::WFormModel::Field field)
  {
    auto it = formDelegates_.find(field);
    if (it != formDelegates_.end()) {
      return it->second;
    }
    return nullptr;
  }
};
    }
  }
}

#endif // WT_FORM_DBO_FORMVIEW
