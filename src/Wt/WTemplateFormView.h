// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2011 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_WTEMPLATE_FORM_VIEW_H_
#define WT_WTEMPLATE_FORM_VIEW_H_

#include <Wt/WTemplate.h>
#include <Wt/WFormModel.h>
#include <Wt/WFormWidget.h>

namespace Wt {

#ifdef WT_TARGET_JAVA
struct Runnable {
  Runnable();
  Runnable(void (*)());
  void run() const;
};
#endif // WT_TARGET_JAVA

/*! \class WTemplateFormView Wt/WTemplateFormView.h
 *  \brief A template-based View class form models.
 *
 * This implements a View to be used in conjunction with WFormModel
 * models to implement forms.
 *
 * For each model field, it uses a number of conventional template placholder
 * variables to represent the label, editor, and validation messages in the
 * template. For a field name '<i>field</i>', we have:
 * - '<i>field</i>': the actual (form) widget for viewing/editing the value
 * - '<i>field</i>-label': the label text
 * - '<i>field</i>-info': a text that contains help or validation messages
 * - 'if:<i>field</i>': condition for the visibility of the field
 *
 * A typical template uses blocks of the following-format (in the example below
 * illustrated for a field 'UserName'):
 *
 * \verbatim
   ${<if:UserName>}
     <label for="${id:UserName}">${UserName-label}</label>
     ${UserName} ${UserName-info}
   ${</if:UserName>}\endverbatim
 *
 * The View may render fields of more than one model, and does not
 * necessarily need to render all information of each model. The latter
 * can be achieved by either calling updateViewField() and updateModelField()
 * for individual model fields, or by hiding fields in the model that are not
 * to be shown in the view.
 *
 * The updateView() method updates the view based on a model (e.g. to
 * propagate changed values or validation feed-back), while the
 * updateModel() method updates the model with values entered in the View.
 *
 * The view is passive: it will not perform any updates by itself of either
 * the View or Model. You will typically bind a method to the StandardButton::Ok button
 * and do:
 * \if cpp
 * \code
 * void MyView::okClicked()
 * {
 *   updateModel(model_);
 *   if (model_->validate()) {
 *     ...
 *   } else {
 *     updateView(model_);
 *   }
 * }
 * \endcode
 * \elseif java
 * \code
 * void okClicked()
 * {
 *   updateModel(this.model);
 *   if (this.model.validate()) {
 *     ...
 *   } else {
 *     updateView(this.model);
 *   }
 * }
 * \endcode
 * \endif
 */
class WT_API WTemplateFormView : public WTemplate
{
public:
  /*! \brief Constructor.
   *
   * For convenience, this initializes the template with:
   * \if cpp
   * \code
   * addFunction("id", &Functions::id);
   * addFunction("tr", &Functions::tr);
   * addFunction("block", &Functions::block);
   * \endcode
   * \elseif java
   * \code
   * addFunction("id", Functions.id);
   * addFunction("tr", Functions.tr);
   * addFunction("block", Functions.block);
   * \endcode
   * \endif
   */
  WTemplateFormView();

  /*! \brief Constructor.
   *
   * For convenience, this initializes the template with:
   * \if cpp
   * \code
   * addFunction("id", &Functions::id);
   * addFunction("tr", &Functions::tr);
   * addFunction("block", &Functions::block);
   * \endcode
   * \elseif java
   * \code
   * addFunction("id", Functions.id);
   * addFunction("tr", Functions.tr);
   * addFunction("block", Functions.block);
   * \endcode
   * \endif
   */
  WTemplateFormView(const WString& text);

  /*! \brief Sets the form widget for a given field.
   *
   * When the \p widget is a form widget, then the View class will use
   * WFormWidget::setValueText() to update it with model values, and
   * WFormWidget::valueText() to update the model with view data.
   *
   * You can override this default behaviour by either using the
   * overloaded setFormWidget() that allows to specify these
   * functions, or reimplement updateViewValue() or updateModelValue().
   */
  void setFormWidget(WFormModel::Field field,
		     std::unique_ptr<WWidget> widget);

  /*! \brief Sets the form widget for a given field.
   *
   * This overloaded functions allows functions to be provided to update the view
   * and model for this field.
   */
  void setFormWidget(WFormModel::Field field,
		     std::unique_ptr<WWidget> widget,
#ifndef WT_TARGET_JAVA
		     const std::function<void ()>& updateViewValue,
		     const std::function<void ()>& updateModelValue
#else // WT_TARGET_JAVA
		     const Runnable& updateViewValue,
		     const Runnable& updateModelValue
#endif // WT_TARGET_JAVA
      );

  /*! \brief Updates the View.
   *
   * This creates or updates all fields in the view.
   *
   * \sa updateViewField(), WFormModel::fields()
   */
  virtual void updateView(WFormModel *model);

  /*! \brief Creates or updates a field in the View.
   *
   * This will update or create and bind widgets in the template to represent
   * the field. To create the form widget that implements the editing, it
   * calls createFormWidget().
   *
   * The default behaviour interprets WFormModel::isVisible(),
   * WFormModel::isReadOnly(), WFormModel::label() and
   * WFormModel::validator() to update the View, and calls
   * updateViewValue() to update the view value. If no form widget has
   * been set for the given \c field using setFormWidget(), then it
   * calls createFormWidget() to try to create one.
   *
   * It's usually more convenient to reimplement updateViewValue() to
   * override specifically how the value from the model should be used
   * to update the form widget.
   */
  virtual void updateViewField(WFormModel *model, WFormModel::Field field);

  /*! \brief Updates the value in the View.
   *
   * The default implementation calls updateViewValue(WFormModel *,
   * WFormField::Field, WWidget *). If this function returned \c
   * false, it sets WFormModel::valueText() into
   * WFormWidget::setValueText().
   */
  virtual void updateViewValue(WFormModel *model, WFormModel::Field field,
			       WFormWidget *edit);

  /*! \brief Updates the value in the View.
   *
   * The default implementation considers only a specialized update
   * function that may have been configured in setFormWidget() and
   * returns \c false if no such function was configured.
   */
  virtual bool updateViewValue(WFormModel *model, WFormModel::Field field,
			       WWidget *edit);

  /*! \brief Updates the Model.
   *
   * This creates or updates all field values in the model.
   *
   * \sa updateModelField(), WFormModel::fields()
   */
  virtual void updateModel(WFormModel *model);

  /*! \brief Updates a field in the Model.
   *
   * This calls updateModelValue() to update the model value.
   */
  virtual void updateModelField(WFormModel *model, WFormModel::Field field);

  /*! \brief Updates a value in the Model.
   *
   * The default implementation calls updateModelValue(WFormModel *,
   * WFormModel::Field, WWidget *), and if that returns \c false, it
   * then calls WFormModel::setValue() with WFormWidget::valueText().
   */
  virtual void updateModelValue(WFormModel *model, WFormModel::Field field,
				WFormWidget *edit);

  /*! \brief Updates a value in the Model.
   *
   * The default implementation considers only a specialized update
   * function that may have been configured in setFormWidget() and
   * returns \c false if no such function was configured.
   */
  virtual bool updateModelValue(WFormModel *model, WFormModel::Field field,
				WWidget *edit);

protected:
  /*! \brief Creates a form widget.
   *
   * This method is called by updateViewField() when it needs to
   * create a form widget for a field, and none was specified using
   * setFormWidget().
   */
  virtual std::unique_ptr<WWidget> createFormWidget(WFormModel::Field field);

  /*! \brief Indicates the validation result.
   *
   * The default implementation calls WTheme::applyValidationStyle()
   *
   * \note We changed the signature to take an edit WWidget instead of
   *       WFormWidget in %Wt 3.3.1!
   */
  virtual void indicateValidation(WFormModel::Field field,
				  bool validated,
				  WText *info,
				  WWidget *edit,
				  const WValidator::Result& validation);
private:
  struct FieldData {
    FieldData();

    WWidget *formWidget;
#ifndef WT_TARGET_JAVA
    std::function<void ()> updateView, updateModel;
#else
    const Runnable *updateView, *updateModel;
#endif
  };

  typedef std::map<std::string, FieldData> FieldMap;
  FieldMap fields_;

  void init();
};

}

#endif // WT_TEMPLATE_FORM_VIEW_H_
