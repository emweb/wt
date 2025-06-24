// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2021 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_FORM_DBO_FORMMODEL_H_
#define WT_FORM_DBO_FORMMODEL_H_

#include <Wt/Form/Dbo/Actions.h>
#include <Wt/Form/Dbo/FieldOptions.h>
#include <Wt/Form/Dbo/FormModelBase.h>

#include <Wt/WSignal.h>

namespace Wt {
  namespace Form {
    namespace Dbo {
/*! \class FormModel Wt/Form/Dbo/FormModel.h Wt/Form/Dbo/FormModel.h
 *  \brief A model class to represent database objects
 *
 * This implements field data and validation handling for database form-based views.
 *
 * The user needs to add all database columns to the model that they want to display
 * in the view before initializing the model. During initialization the model will
 * read the information from the database.
 *
 * \sa FormView
 *
 * \ingroup form
 */
template<class C>
class FormModel : public FormModelBase
{
public:
  /*! \brief Constructor
   *
   * Creates a new form model
   */
  explicit FormModel(Wt::Dbo::Session& session, Wt::Dbo::ptr<C> ptr = nullptr)
    : FormModelBase(),
      session_(session),
      item_(ptr)
  {
  }

  /*! \brief Adds a %Dbo column as a field
   *
   * \throws WException when the method is called after initDboValues
   */
  void addDboField(Wt::WFormModel::Field field)
  {
    if (valuesInitialized_) {
      throw Wt::WException("Cannot add Dbo field after values have been initialized!");
    }

    insertDboField(field);
  }

  /*! \brief Adds a list of %Dbo columns to the model
   *
   * \throws WException when the method is called after initDboValues
   */
  void addDboFields(std::vector<Wt::WFormModel::Field> fields)
  {
    if (valuesInitialized_) {
      throw Wt::WException("Cannot add Dbo fields after values have been initialized!");
    }

    for (const Wt::WFormModel::Field& f : fields) {
      insertDboField(f);
    }
  }

  /*! \brief Adds all %Dbo columns to the model
   *
   * \throws WException when the method is called after initDboValues
   */
  void addAllDboColumnsAsFields(Wt::WFlags<FieldOptions> options = Wt::WFlags<FieldOptions>())
  {
    if (valuesInitialized_) {
      throw Wt::WException("Cannot add Dbo fields after values have been initialized!");
    }

    C dummy;

    dboFields_.clear();
    ModelAction action(session_, dboFields_, options);
    dummy.persist(action);

    for (const std::string& f : dboFields_) {
      addField(f.c_str());
    }
  }

  /*! \brief Initializes the model with values from the database
   */
  void initDboValues()
  {
    LoadAction action(session_, this);

    if (item_) {
      Wt::Dbo::Transaction t(session_);
      const_cast<C*>(item_.get())->persist(action);
    } else {
      // If the model is used to add a new item,
      // preload the model with the default values for the class
      C dummy;
      dummy.persist(action);
    }

    valuesInitialized_ = true;
  }

  /*! \brief Saves the changes made in the model to the database
   */
  void saveDboValues()
  {
    Wt::Dbo::Transaction t(session_);
    if (!item_) {
      item_ = session_.add(std::make_unique<C>());
    }

    SaveAction action(session_, this);
    item_.modify()->persist(action);
  }

  /*! \brief Validates a field
   */
  bool validateField(Wt::WFormModel::Field field) override
  {
    Wt::cpp17::any v = value(field);
    if (v.type() == typeid(Wt::WValidator::Result)) {
      // We need to use setValidation, otherwise the view will color the field red,
      // but it won't say why the entered value is wrong.
      Wt::WValidator::Result result = Wt::cpp17::any_cast<Wt::WValidator::Result>(v);
      setValidation(field, result);
      return false;
    }

    return Wt::WFormModel::validateField(field);
  }

  /*! \brief Returns the Session object used by this class
   */
  Wt::Dbo::Session& session() const { return session_; }

  /*! \brief Sets the item of the model
   * 
   * Changes the item of the model. This will emit the itemChanged() signal,
   * making the FormView update.
   * 
   * Setting the item to \p nullptr will result in a new item being
   * constructed. This will load the default values of the item. The
   * item remains transient (ptr::isTransient()), until it is saved,
   * using saveDboValues() or FormView::save().
   */
  void setItem(Wt::Dbo::ptr<C> ptr)
  {
    item_ = ptr;
    initDboValues();
    itemChanged_.emit();
  }

  /*! \brief Returns the database object used by this class
   *
   * \sa setItem()
   */
  const Wt::Dbo::ptr<C>& item() const { return item_; }

  /*! \brief Signal emited when the item is changed
   * 
   * Signal emitted when setItem() is called.
   * 
   * If this model is used as the model of a FormView, (using
   * FormView::setModel()), the FormView will listen to this signal and
   * update when it is emitted.
   */
  Signal<>& itemChanged() { return itemChanged_; }

private:
  Wt::Dbo::Session& session_;
  Wt::Dbo::ptr<C> item_;
  Wt::Signal<> itemChanged_;

  bool valuesInitialized_ = false;
};
    }
  }
}

#endif // WT_FORM_DBO_FORMMODEL
