// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2021 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef TEST_DBO_OBJECT_H_
#define TEST_DBO_OBJECT_H_

#include <Wt/Dbo/Types.h>
#include <Wt/Dbo/WtSqlTraits.h>

#include <Wt/WDate.h>
#include <Wt/WDateTime.h>
#include <Wt/WString.h>
#include <Wt/WTime.h>

#include "CustomSqlTraits.h"

class TestDboPtr : public Wt::Dbo::Dbo<TestDboPtr>
{
public:
  TestDboPtr();
  explicit TestDboPtr(const Wt::WString& name);

  template<class Action>
  void persist(Action& a)
  {
    Wt::Dbo::field(a, name_, "name");
  }

private:
  Wt::WString name_;
};

DBO_EXTERN_TEMPLATES(TestDboPtr)

class TestDboObject : public Wt::Dbo::Dbo<TestDboObject>
{
public:
  enum class Enum {
    Unknown = 0,
    Value1 = 1,
    Value2 = 2,
    Value3 = 3,
    LAST_ELEMENT
  };

  TestDboObject();

  template<class Action>
  void persist(Action& a)
  {
    Wt::Dbo::field(a, integerValue_, "int_value");
    Wt::Dbo::field(a, doubleValue_, "double_value");
    Wt::Dbo::field(a, booleanValue_, "bool_value");
    Wt::Dbo::field(a, enumValue_, "enum_value");
    Wt::Dbo::field(a, stdStringValue_, "std_string_value");
    Wt::Dbo::field(a, stringValue_, "string_value");
    Wt::Dbo::field(a, otherStringValue_, "other_string_value");
    Wt::Dbo::field(a, dateValue_, "date_value");
    Wt::Dbo::field(a, timeValue_, "time_value");
    Wt::Dbo::field(a, dateTimeValue_, "date_time_value");
    Wt::Dbo::field(a, textValue_, "text_value");

    Wt::Dbo::belongsTo(a, ptr_, "ptr", Wt::Dbo::OnDeleteSetNull);

    Wt::Dbo::hasMany(a, collection_, Wt::Dbo::ManyToMany, "ptr_collection", "test_dbo_object", Wt::Dbo::OnDeleteCascade);
  }

private:
  int integerValue_ = 0;
  double doubleValue_ = 0.0;
  bool booleanValue_ = false;

  Enum enumValue_ = Enum::Unknown;

  std::string stdStringValue_ = "";
  Wt::WString stringValue_;
  Wt::WString otherStringValue_;
  Wt::WDate dateValue_;
  Wt::WTime timeValue_;
  Wt::WDateTime dateTimeValue_;

  Text textValue_;

  Wt::Dbo::ptr<TestDboPtr> ptr_;
  Wt::Dbo::collection<Wt::Dbo::ptr<TestDboPtr>> collection_;
};

DBO_EXTERN_TEMPLATES(TestDboObject)

#endif // TEST_DBO_OBJECT_H_
