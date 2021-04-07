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

#include <Wt/Form/Dbo/FormModelBase.h>

namespace Wt {
  namespace Form {
    namespace Dbo {

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

/*! \brief Class that automatically loads data from the database
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

  /*
   * This function will be called when Wt::Dbo::id is used
   * in the persist method when specifying a natural primary key
   */
  template<typename V>
  void actId(V& value, const std::string& name, int size)
  {
    if (hasDboField(name)) {
      model()->setValue(name.c_str(), value);
    }
  }

  /*
   * This function will be called when Wt::Dbo::id is used in
   * the persist method when specifying a natural primary key
   */
  template<class C>
  void actId(Wt::Dbo::ptr<C>& value, const std::string& name, int size, int fkConstraints)
  {
    if (hasDboField(name)) {
      model()->setValue(name.c_str(), value);
    }
  }

  /*
   * Assumes that the WFormModel can handle the data types
   * that are used in the Dbo class like WString, WDate, ..
   */
  template<typename V>
  void act(const Wt::Dbo::FieldRef<V>& ref)
  {
    if (hasDboField(ref.name())) {
      model()->setValue(ref.name().c_str(), ref.value());
    }
  }

  /*
   * Assumes that the WFormModel can handle Wt::Dbo::ptr
   */
  template<class C>
  void actPtr(const Wt::Dbo::PtrRef<C>& ref)
  {
    if (hasDboField(ref.name())) {
      model()->setValue(ref.name().c_str(), ref.value());
    }
  }

  /*
   * Assumes that the WFormModel can handle Wt::Dbo::ptr
   */
  template<class C>
  void actWeakPtr(const Wt::Dbo::WeakPtrRef<C>& ref)
  {
    if (hasDboField(ref.joinName())) {
      model()->setValue(ref.joinName().c_str(), ref.value());
    }
  }

  /*
   * Assumes that the WFormModel can handle a std::vector
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

/*! \brief Class that automatically saves data from the form to the database
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

  /*
   * This function will be called when Wt::Dbo::id is used
   * in the persist method when specifying a natural primary key
   */
  template<typename V>
  void actId(V& value, const std::string& name, int size)
  {
    if (hasDboField(name)) {
      value = Wt::cpp17::any_cast<V>(model()->value(name.c_str()));
    }
  }

  /*
   * This function will be called when Wt::Dbo::id is used
   * in the persist method when specifying a natural primary key
   */
  template<class C>
  void actId(Wt::Dbo::ptr<C>& value, const std::string& name, int size, int fkConstraints)
  {
    if (hasDboField(name)) {
      value = Wt::cpp17::any_cast<Wt::Dbo::ptr<C>>(model()->value(name.c_str()));
    }
  }

  /*
   * Assumes that the WFormModel returns the data types expected
   * by the Dbo class like WString, WDate, ..
   */
  template<typename V>
  void act(const Wt::Dbo::FieldRef<V>& ref)
  {
    if (hasDboField(ref.name())) {
      ref.setValue(Wt::cpp17::any_cast<V>(model()->value(ref.name().c_str())));
    }
  }

  /*
   * Assumes that the WFormModel returns a Wt::Dbo::ptr
   */
  template<class C>
  void actPtr(const Wt::Dbo::PtrRef<C>& ref)
  {
    if (hasDboField(ref.name())) {
      ref.value() = Wt::cpp17::any_cast<Wt::Dbo::ptr<C>>(model()->value(ref.name().c_str()));
    }
  }

  /*
   * Assumes that the WFormModel returns a Wt::Dbo::ptr
   */
  template<class C>
  void actWeakPtr(const Wt::Dbo::WeakPtrRef<C>& ref)
  {
    if (hasDboField(ref.joinName())) {
      ref.value() = Wt::cpp17::any_cast<Wt::Dbo::ptr<C>>(model()->value(ref.joinName().c_str()));
    }
  }

  /*
   * Assumes that the WFormModel returns a std::vector
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
    }
  }
}

#endif // WT_FORM_DBO_ACTIONS_
