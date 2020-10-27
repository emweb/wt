// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_WFORM_MODEL_H_
#define WT_WFORM_MODEL_H_

#include <Wt/WAny.h>
#include <Wt/WObject.h>
#include <Wt/WValidator.h>

#include <cstring>

namespace Wt {

/*! \class WFormModel Wt/WFormModel.h
 *  \brief A basic model class for forms.
 *
 * This implements field data and validation handling for (simple)
 * form-based views. It provides a standard way for views to perform
 * field validation, and react to validation results.
 *
 * All fields are uniquely identified using a string literal (which is
 * the Field type). For each field, its value, the visibility, whether the
 * field is read-only, and its current validation status is managed by the
 * model. In addition, you will typically specialize the class to customize
 * the validation and application logic.
 *
 * \if cpp
 * \warning The Field type is 'const char *, and instead of string
 *          comparisons to identify fields, pointer comparisons are used.
 *          Thus you really should use the same field constant throughout
 *          your code to refer to a given field, and you cannot use
 *          constexpr for the field constant since that does not have a unique
 *          value (since it has no storage).
 * \endif
 *
 * Although it can be setup to use WValidator objects for individual
 * fields, also other validation where more entered information needs
 * to be considered simultaneously can be implemented.
 *
 * A model is typically used by a View which renders the fields configured
 * in the model, updates the model values, invokes and reflects the validation
 * status.
 *
 * Example (a bit contrived since you will usually not use the model directly):
 * \if cpp
 * \code
 * Wt::WFormModel::Field NameField = "name";
 * Wt::WFormModel::Field TelField = "telephone";
 *
 * auto model = std::make_shared<Wt::WFormModel>();
 * model->addField(NameField, "Enter your name");
 * model->addField(TelField, "Phone number");
 *
 * model->setValue(NameField, Wt::WString::fromUTF8("John Doe"));
 *
 * if (model->validate()) {
 *   ...
 * } else {
 *   const Wt::WValidator::Result& rname = model->validation(NameField);
 *   if (rname.state() != Wt::ValidationState::Valid) {
 *     std::cerr <<< "Invalid name: " << rname.message();
 *   }
 *   ...
 * }
 * \endcode
 * \elseif java
 * \code
 * String NameField = "name";
 * String TelField = "telephone";
 *
 * WFormModel model = new WFormModel();
 * model.addField(NameField, "Enter your name");
 * model.addField(TelField, "Phone number");
 *
 * model.setValue(NameField, "John Doe");
 *
 * if (model.validate()) {
 *   ...
 * } else {
 *   WValidator.Result rname = model.getValidation(NameField);
 *   if (rname.getState() != WValidator.State.Valid) {
 *     System.err.println("Invalid name: " + rname.getMessage());
 *   }
 *   ...
 * }
 * \endcode
 * \endif
 */
class WT_API WFormModel : public WObject
{
public:
  /*! \brief A type to identify a field.
   *
   * Fields are identified by a string literal constant.
   */
  typedef const char *Field;

  /*! \brief Constructor.
   *
   * Creates a new form model.
   */
  WFormModel();

  /*! \brief Adds a field.
   *
   * The \p field is added to the model, with an optional short
   * informational message that can be used by views to provide a hint
   * on the value that needs to be entered. The message is set as the
   * validation message as long as the field has not yet been
   * validated.
   *
   * If the \p field was already in the model, its data is reset.
   *
   * \if cpp
   * Note that Field is a const char *. In versions of Wt before 4.5.0,
   * a field would be identified by a string literal and pointer comparison
   * would be used. In Wt 4.5.0 this has changed to string comparison.
   * However, you should still make sure the \p field argument to this
   * function should either be a string literal like before, or otherwise
   * outlive the %WFormModel. Ideally, Field would simply become a std::string
   * in the future.
   * \endif
   */
  void addField(Field field, const WString& info = WString::Empty);

  /*! \brief Removes a field.
   *
   * The \p field is removed from the model.
   */
  void removeField(Field field);

  /*! \brief Returns the fields.
   *
   * This returns the fields currently configured in the model (added with
   * addField() or for which a value or property has been set).
   */
  std::vector<Field> fields() const;

  /*! \brief Resets the model.
   *
   * The default implementation clears the value of all fields, and resets
   * the validation state to not validated.
   */
  virtual void reset();

  /*! \brief Validates the current input.
   *
   * The default implementation calls validateField() for each field and
   * returns \c true if all fields validated.
   *
   * \sa validateField()
   */
  virtual bool validate();

  /*! \brief Returns the current overall validation state.
   *
   * This checks the validation() of all fields, and returns \c true if all
   * all fields have been validated and are valid.
   *
   * \sa validate()
   */
  bool valid() const;

  /*! \brief Sets whether a field is visible.
   *
   * Fields are visible by default. An invisible field will be ignored during
   * validation (i.e. will be considered as valid).
   *
   * \sa isVisible()
   */
  void setVisible(Field field, bool visible);

  /*! \brief Returns whether a field is visible.
   *
   * In some cases not all fields of the model need to be shown. This
   * may depend on values input for certain fields, and thus change
   * dynamically. You may specialize this method to indicate that a certain
   * field should be invisible.
   *
   * The default implementation returns the value set by setVisible().
   */
  virtual bool isVisible(Field field) const;

  /*! \brief Sets whether a field is read-only.
   *
   * Fields are read-write by default.
   *
   * \sa isReadOnly()
   */
  void setReadOnly(Field field, bool readOnly);

  /*! \brief Returns whether a field is read only.
   *
   * The default implementation returns the value set by setReadOnly()
   */
  virtual bool isReadOnly(Field field) const;

  /*! \brief Returns a field label.
   *
   * The default implementation returns the WString::tr(field)
   */
  virtual WString label(Field field) const;

  /*! \brief Sets the field value.
   *
   * \sa value(), valueText()
   */
  virtual void setValue(Field field, const cpp17::any& value);

  /*! \brief Returns the field value.
   *
   * \sa valueText(), setValue()
   */
  virtual const cpp17::any& value(Field field) const;

  /*! \brief Returns the field value text.
   *
   * \if cpp
   * This uses Wt::asString() to interpret the current value as text.
   * \endif
   *
   * \sa value()
   */
  virtual WT_USTRING valueText(Field field) const;

  /*! \brief Sets a validator.
   */
  virtual void setValidator(Field field,
			    const std::shared_ptr<WValidator>& validator);

  /*! \brief Returns a validator.
   *
   * Returns the validator for the field.
   */
  virtual std::shared_ptr<WValidator> validator(Field field) const;

  /*! \brief Validates a field.
   *
   * The default implementation uses the validator configured for the
   * field to validate the field contents, or if no validator has been
   * configured assumes that the field is valid.
   *
   * You will typically customize this method for more complex validation
   * cases.
   *
   * \sa validate(), validationResult()
   */
  virtual bool validateField(Field field);

  /*! \brief Sets whether a field has been validated.
   *
   * This is usually not used directly, but invoked by setValidation()
   *
   * A field is initially (or after reset()), not validated.
   */
  virtual void setValidated(Field field, bool validated);

  /*! \brief Returns whether the field has been validated yet.
   *
   * This is initially \c false, and set to \c true by setValidation().
   *
   * \sa setValidated()
   */
  virtual bool isValidated(Field field) const;

  /*! \brief Returns the result of a validation.
   *
   * \sa validateField()
   */
  const WValidator::Result& validation(Field field) const;

  /*! \brief Sets the validation result for a field.
   *
   * This will also set the field as validated.
   *
   * \sa validation(), isValidated()
   */
  virtual void setValidation(Field field, const WValidator::Result& result);

private:
  struct FieldData {
    FieldData();

    std::shared_ptr<WValidator> validator;
    cpp17::any value;
    WValidator::Result validation;
    bool visible, readOnly, validated;
  };

#ifndef WT_TARGET_JAVA
  struct FieldComparator {
      bool operator()(const char *left, const char *right) const
      {
        return std::strcmp(left, right) < 0;
      }
  };

  typedef std::map<Field, FieldData, FieldComparator> FieldMap;
#else
  typedef std::map<Field, FieldData> FieldMap;
#endif

  FieldMap fields_;

  static const WValidator::Result Valid;
  static const cpp17::any NoValue;
};

}

#endif // WT_WFORM_MODEL_H_
