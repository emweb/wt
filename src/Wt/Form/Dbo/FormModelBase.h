// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2021 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_FORM_DBO_FORMMODELBASE_H_
#define WT_FORM_DBO_FORMMODELBASE_H_

#include <Wt/WFormModel.h>

namespace Wt {
  namespace Form {
    namespace Dbo {
/*! \brief A model class keeping information about database columns
 * that are to be used in the model/view logic.
 */
class FormModelBase : public Wt::WFormModel
{
public:
  FormModelBase()
    : Wt::WFormModel()
  {
  }

  /*! \brief Returns the Dbo columns used in this model
   */
  const std::vector<std::string>& dboFields() const { return dboFields_; }

protected:
  std::vector<std::string> dboFields_;

  /*! \brief Adds a Dbo column as a field
   */
  void insertDboField(Wt::WFormModel::Field field)
  {
    dboFields_.push_back(field);
    addField(field);
  }
};
    }
  }
}

#endif // WT_FORM_DBO_FORMMODELBASE
