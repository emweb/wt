// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2021 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_FORM_DBO_ACTIONS_H_
#define WT_FORM_DBO_ACTIONS_H_

#include <algorithm>

#include <Wt/Dbo/Session.h>

#include <Wt/Form/Dbo/FieldOptions.h>
#include <Wt/Form/Dbo/FormModelBase.h>
#include <Wt/Form/WFormDelegate.h>

namespace Wt {
  namespace Form {
    /*! \brief Namespace that contain %Dbo specific classes within the Form module
     */
    namespace Dbo {

/*! \internal
 *  \class Action Wt/Form/Dbo/Actions.h Wt/Form/Dbo/Actions.h
 *  \brief Base class for customized actions
 */
class Action
{
public:
  Action(Wt::Dbo::Session& session, FormModelBase *model)
    : session_(session),
      model_(model)
  {
  }

  Wt::Dbo::Session* session() const { return &session_; }

protected:
  FormModelBase* model() const { return model_; }

  /*! \brief Returns whether or not the database column is used in the model
   *
   * We need to make sure the database column is actually used in the
   * model before storing the information in the WFormModel. Otherwise
   * WFormModel::setValue will add the column to the model, which is not
   * what we want here.
   */
  bool hasDboField(const std::string& name)
  {
    const auto &fields = model()->dboFields();
    return std::find(begin(fields), end(fields), name) != end(fields);
  }

private:
  Wt::Dbo::Session& session_;
  FormModelBase *model_;
};

/*! \internal
 *  \class LoadAction Wt/Form/Dbo/Actions.h Wt/Form/Dbo/Actions.h
 *  \brief Class that automatically loads data from the database
 *
 * This class will be used together with Dbo's persist method to
 * automatically initialize the WFormModel with data from the database
 *
 * Data is loaded from the database, it's not modified.
 */
class LoadAction : public Action
{
public:
  LoadAction(Wt::Dbo::Session& session, FormModelBase *model)
    : Action(session, model)
  {
  }

  /*! \note This function will be called when Wt::Dbo::id is used
   * in the persist method when specifying a natural primary key
   */
  template<typename V>
  void actId(V& value, const std::string& name, int size)
  {
    if (hasDboField(name)) {
      model()->setValue(name.c_str(), value);
    }
  }

  /*! \note This function will be called when Wt::Dbo::id is used in
   * the persist method when specifying a natural primary key
   */
  template<class C>
  void actId(Wt::Dbo::ptr<C>& value, const std::string& name, int size, int fkConstraints)
  {
    if (hasDboField(name)) {
      model()->setValue(name.c_str(), value);
    }
  }

  /*! \note Assumes that the WFormModel can handle the data types
   * that are used in the Dbo class like WString, WDate, ...
   */
  template<typename V>
  void act(const Wt::Dbo::FieldRef<V>& ref)
  {
    if (hasDboField(ref.name())) {
      model()->setValue(ref.name().c_str(), ref.value());
    }
  }

  /*! \note Assumes that the WFormModel can handle Wt::Dbo::ptr
   */
  template<class C>
  void actPtr(const Wt::Dbo::PtrRef<C>& ref)
  {
    if (hasDboField(ref.name())) {
      model()->setValue(ref.name().c_str(), ref.value());
    }
  }

  /*! \note Assumes that the WFormModel can handle Wt::Dbo::ptr
   */
  template<class C>
  void actWeakPtr(const Wt::Dbo::WeakPtrRef<C>& ref)
  {
    if (hasDboField(ref.joinName())) {
      model()->setValue(ref.joinName().c_str(), ref.value());
    }
  }

  /*! \note Assumes that the WFormModel can handle a std::vector
   * of Wt::Dbo::ptr objects.
   */
  template<class C>
  void actCollection(const Wt::Dbo::CollectionRef<C>& ref)
  {
    if (hasDboField(ref.joinName())) {
      Wt::Dbo::Transaction t(*session());
      model()->setValue(ref.joinName().c_str(), std::vector<Wt::Dbo::ptr<C>>(ref.value().begin(), ref.value().end()));
    }
  }

  bool getsValue() const { return true; }
  bool setsValue() const { return false; }
  bool isSchema() const { return false; }
};

/*! \internal
 *  \class SaveAction Wt/Form/Dbo/Actions.h Wt/Form/Dbo/Actions.h
 *  \brief Class that automatically saves data from the form to the database
 *
 * This class will be used together with Dbo's persist method to automatically
 * push the data from the WFormModel to the database.
 *
 * This action modifies the data in the database.
 */
class SaveAction : public Action
{
public:
  SaveAction(Wt::Dbo::Session& session, FormModelBase *model)
    : Action(session, model)
  {
  }

  /*! \note This function will be called when Wt::Dbo::id is used
   * in the persist method when specifying a natural primary key
   */
  template<typename V>
  void actId(V& value, const std::string& name, int size)
  {
    if (hasDboField(name)) {
      value = Wt::cpp17::any_cast<V>(model()->value(name.c_str()));
    }
  }

  /*! \note This function will be called when Wt::Dbo::id is used
   * in the persist method when specifying a natural primary key
   */
  template<class C>
  void actId(Wt::Dbo::ptr<C>& value, const std::string& name, int size, int fkConstraints)
  {
    if (hasDboField(name)) {
      value = Wt::cpp17::any_cast<Wt::Dbo::ptr<C>>(model()->value(name.c_str()));
    }
  }

  /*! \note Assumes that the WFormModel returns the data types expected
   * by the Dbo class like WString, WDate, ...
   */
  template<typename V>
  void act(const Wt::Dbo::FieldRef<V>& ref)
  {
    if (hasDboField(ref.name())) {
      ref.setValue(Wt::cpp17::any_cast<V>(model()->value(ref.name().c_str())));
    }
  }

  /*! \note Assumes that the WFormModel returns a Wt::Dbo::ptr
   */
  template<class C>
  void actPtr(const Wt::Dbo::PtrRef<C>& ref)
  {
    if (hasDboField(ref.name())) {
      ref.value() = Wt::cpp17::any_cast<Wt::Dbo::ptr<C>>(model()->value(ref.name().c_str()));
    }
  }

  /*! \note Assumes that the WFormModel returns a Wt::Dbo::ptr
   */
  template<class C>
  void actWeakPtr(const Wt::Dbo::WeakPtrRef<C>& ref)
  {
    if (hasDboField(ref.joinName())) {
      ref.value() = Wt::cpp17::any_cast<Wt::Dbo::ptr<C>>(model()->value(ref.joinName().c_str()));
    }
  }

  /*! \note Assumes that the WFormModel returns a std::vector
   * of Wt::Dbo::ptr objects
   */
  template<class C>
  void actCollection(const Wt::Dbo::CollectionRef<C>& ref)
  {
    if (hasDboField(ref.joinName())) {
      Wt::Dbo::Transaction t(*session());

      ref.value().clear();

      std::vector<Wt::Dbo::ptr<C>> values = Wt::cpp17::any_cast<std::vector<Wt::Dbo::ptr<C>>>(model()->value(ref.joinName().c_str()));
      for (const Wt::Dbo::ptr<C>& v : values) {
        ref.value().insert(v);
      }
    }
  }

  bool getsValue() const { return false; }
  bool setsValue() const { return true; }
  bool isSchema() const { return false; }
};

/*! \internal
 *  \class ViewAction Wt/Form/Dbo/Actions.h Wt/Form/Dbo/Actions.h
 *  \brief Class that automatically initializes the WFormDelegates to be used by the View
 *
 * This class will automatically generate the WFormDelegates based
 * on the data type that's used in the Dbo class.
 *
 * This action neither loads nor sets data in the database
 */
class ViewAction : public Action
{
public:
  ViewAction(Wt::Dbo::Session& session, FormModelBase *model,
             std::map<std::string, std::shared_ptr<Wt::Form::WAbstractFormDelegate>>& formDelegates)
    : Action(session, model),
      formDelegates_(formDelegates)
  {
  }

  template<typename V>
  void actId(V& value, const std::string& name, int size)
  {
    if (!hasDelegate(name) && hasDboField(name)) {
      insertFormDelegate(name, std::make_shared<Wt::Form::WFormDelegate<V>>());
    }
  }

  template<class C>
  void actId(Wt::Dbo::ptr<C>& value, const std::string& name, int size, int fkConstraints)
  {
    if (!hasDelegate(name) && hasDboField(name)) {
      throw Wt::WException("Default WFormDelegate is not yet implemented for Wt::Dbo::ptr");
    }
  }

  template<typename V>
  void act(const Wt::Dbo::FieldRef<V>& ref)
  {
    if (!hasDelegate(ref.name()) && hasDboField(ref.name())) {
      insertFormDelegate(ref.name(), std::make_shared<Wt::Form::WFormDelegate<V>>());
    }
  }

  template<class C>
  void actPtr(const Wt::Dbo::PtrRef<C>& ref)
  {
    if (!hasDelegate(ref.name()) && hasDboField(ref.name())) {
      throw Wt::WException("Default WFormDelegate is not yet implemented for Wt::Dbo::ptr");
    }
  }

  template<class C>
  void actWeakPtr(const Wt::Dbo::WeakPtrRef<C>& ref)
  {
    if (!hasDelegate(ref.joinName()) && hasDboField(ref.joinName())) {
      throw Wt::WException("Default WFormDelegate is not yet implemented for Wt::Dbo::ptr");
    }
  }

  template<class C>
  void actCollection(const Wt::Dbo::CollectionRef<C>& ref)
  {
    if (!hasDelegate(ref.joinName()) && hasDboField(ref.joinName())) {
      throw Wt::WException("Default WFormDelegate is not yet implemented for Wt::Dbo::collection");
    }
  }

  bool getsValue() const { return false; }
  bool setsValue() const { return false; }
  bool isSchema() const { return false; }

private:
  std::map<std::string, std::shared_ptr<Wt::Form::WAbstractFormDelegate>>& formDelegates_;

  void insertFormDelegate(const std::string& name, std::shared_ptr<Wt::Form::WAbstractFormDelegate> delegate)
  {
    formDelegates_[name] = delegate;
  }

  /*
   * Check if a custom delegate has been specified
   * for this column
   */
  bool hasDelegate(const std::string& name)
  {
    return formDelegates_.find(name) != formDelegates_.end();
  }
};

/*! \internal
 *  \class ModelAction Wt/Form/Dbo/Actions.h Wt/Form/Dbo/Actions.h
 *  \brief Class that automatically adds all database columns to the Model
 *
 * This action neither loads nor sets data in the database
 */
class ModelAction
{
public:
  ModelAction(Wt::Dbo::Session& session, std::vector<std::string>& fields, Wt::WFlags<FieldOptions> options)
    : session_(session),
      fields_(fields),
      options_(options)
  {
  }

  template<typename V>
  void actId(V& value, const std::string& name, int size)
  {
    insertField(name);
  }

  template<class C>
  void actId(Wt::Dbo::ptr<C>& value, const std::string& name, int size, int fkConstraints)
  {
    if (!options_.test(FieldOptions::ExcludeForeignKeys)) {
      insertField(name);
    }
  }

  template<typename V>
  void act(const Wt::Dbo::FieldRef<V>& ref)
  {
    insertField(ref.name());
  }

  template<class C>
  void actPtr(const Wt::Dbo::PtrRef<C>& ref)
  {
    if (!options_.test(FieldOptions::ExcludeForeignKeys)) {
      insertField(ref.name());
    }
  }

  template<class C>
  void actWeakPtr(const Wt::Dbo::WeakPtrRef<C>& ref)
  {
    if (!options_.test(FieldOptions::ExcludeForeignKeys)) {
      insertField(ref.joinName());
    }
  }

  template<class C>
  void actCollection(const Wt::Dbo::CollectionRef<C>& ref)
  {
    if (!options_.test(FieldOptions::ExcludeForeignKeys)) {
      insertField(ref.joinName());
    }
  }

  bool getsValue() const { return false; }
  bool setsValue() const { return false; }
  bool isSchema() const { return false; }

  Wt::Dbo::Session* session() const { return &session_; }

private:
  Wt::Dbo::Session& session_;
  std::vector<std::string>& fields_;
  Wt::WFlags<FieldOptions> options_;

  void insertField(const std::string& name)
  {
    fields_.push_back(name);
  }
};
    }
  }
}

#endif // WT_FORM_DBO_ACTIONS_
