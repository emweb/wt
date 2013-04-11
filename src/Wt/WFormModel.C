/*
 * Copyright (C) 2011 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WBoostAny>
#include <Wt/WFormModel>
#include <Wt/WLogger>

namespace Wt {

LOGGER("WFormModel");

const WValidator::Result WFormModel::Valid(WValidator::Valid, WString());
const boost::any WFormModel::NoValue;

WFormModel::FieldData::FieldData()
  : validator(0),
    visible(true),
    readOnly(false),
    validated(false)
{ 
  //TODO
  //this is a workaround for cnor, 
  //because cnor seems to be unable to map initalizer ctors
  //fix this in cnor
  value = boost::any();
}

WFormModel::WFormModel(WObject *parent)
  : WObject(parent)
{ }

void WFormModel::addField(Field field, const WString& info)
{
  fields_[field] = FieldData();
  fields_[field].validation = WValidator::Result(WValidator::Invalid, info);
}

void WFormModel::removeField(Field field)
{
  fields_.erase(field);
}

std::vector<WFormModel::Field> WFormModel::fields() const
{
  std::vector<WFormModel::Field> result;

  for (FieldMap::const_iterator i = fields_.begin(); i != fields_.end(); ++i)
    result.push_back(i->first);

  return result;
}

void WFormModel::setReadOnly(Field field, bool readOnly)
{
  FieldMap::iterator i = fields_.find(field);

  if (i != fields_.end())
    i->second.readOnly = readOnly;
  else
    LOG_ERROR("setReadOnly(): " << field << " not in model");
}

bool WFormModel::isReadOnly(Field field) const
{
  FieldMap::const_iterator i = fields_.find(field);

  if (i != fields_.end())
    return i->second.readOnly;
  else
    return false;
}

void WFormModel::setVisible(Field field, bool visible)
{
  FieldMap::iterator i = fields_.find(field);

  if (i != fields_.end())
    i->second.visible = visible;
  else
    LOG_ERROR("setVisible(): " << field << " not in model");
}

bool WFormModel::isVisible(Field field) const
{
  FieldMap::const_iterator i = fields_.find(field);

  if (i != fields_.end())
    return i->second.visible;
  else
    return true;
}

WString WFormModel::label(Field field) const
{
  return WString::tr(field);
}

void WFormModel::setValue(Field field, const boost::any& value)
{
  fields_[field].value = value;
}

const boost::any& WFormModel::value(Field field) const
{
  FieldMap::const_iterator i = fields_.find(field);

  if (i != fields_.end())
    return i->second.value;
  else
    return NoValue;
}

WT_USTRING WFormModel::valueText(Field field) const
{
  WValidator *v = validator(field);

  return asString(value(field), v ? v->format() : WT_USTRING());
}

void WFormModel::setValidator(Field field, WValidator *validator)
{
  FieldMap::iterator i = fields_.find(field);

  if (i != fields_.end()) {
    FieldData& d = i->second;
    if (d.validator && d.validator->parent() == this)
      delete d.validator;

    d.validator = validator;

    if (validator && !validator->parent())
      addChild(validator);
  } else
    LOG_ERROR("setValidator(): " << field << " not in model");
}

WValidator *WFormModel::validator(Field field) const
{
  FieldMap::const_iterator i = fields_.find(field);

  if (i != fields_.end()) {
    const FieldData& d = i->second;

    return d.validator;
  }

  return 0;
}

bool WFormModel::validateField(Field field)
{
  if (!isVisible(field))
    return true;

  FieldMap::iterator i = fields_.find(field);

  if (i != fields_.end()) {
    FieldData& d = i->second;

    if (d.validator)
      setValidation(field, d.validator->validate(asString(valueText(field))));
    else
      setValidation(field, Valid);

    return d.validation.state() == WValidator::Valid;
  } else
    return true;
}

void WFormModel::reset()
{
  for (FieldMap::iterator i = fields_.begin(); i != fields_.end(); ++i) {
    i->second.value = boost::any();
    i->second.validated = false;
  }
}

bool WFormModel::valid() const
{
  for (FieldMap::const_iterator i = fields_.begin(); i != fields_.end(); ++i) {
    const FieldData& fd = i->second;
    
    if (!fd.visible)
      continue;

    if (!fd.validated
	|| fd.validation.state() != WValidator::Valid)
      return false;
  }

  return true;
}

bool WFormModel::validate()
{
  bool result = true;

  for (FieldMap::iterator i = fields_.begin(); i != fields_.end(); ++i) {
    if (!validateField(i->first))
      result = false;
  }

  return result;
}

bool WFormModel::isValidated(Field field) const
{
  FieldMap::const_iterator i = fields_.find(field);

  if (i != fields_.end())
    return i->second.validated;
  else
    return false;
}

void WFormModel::setValidated(Field field, bool validated)
{
  FieldMap::iterator i = fields_.find(field);

  if (i != fields_.end())
    i->second.validated = validated;
  else
    LOG_ERROR("setValidated(): " << field << " not in model");
}

void WFormModel::setValidation(Field field,
			       const WValidator::Result& result)
{
  FieldMap::iterator i = fields_.find(field);

  if (i != fields_.end()) {
    i->second.validation = result;
    setValidated(field, true);
  } else
    LOG_ERROR("setValidation(): " << field << " not in model");
}

const WValidator::Result& WFormModel::validation(Field field) const
{
  FieldMap::const_iterator i = fields_.find(field);

  if (i != fields_.end())
    return i->second.validation;
  else
    return Valid;
}

}
