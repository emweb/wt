// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2021 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_FORM_FORM_DELEGATE_H_
#define WT_FORM_FORM_DELEGATE_H_

#include <Wt/Form/WAbstractFormDelegate.h>

namespace Wt {
  namespace Form {

template<typename T, class Enable = void>
class WFormDelegate;

/*! \brief %Form delegate class for WString
 *
 * This will create a WLineEdit to display the WString value
 * in the View
 *
 * \ingroup form
 */
template<>
class WT_API WFormDelegate<Wt::WString, void> : public WAbstractFormDelegate
{
public:
  /*! \brief Create a form delegate
   */
  WFormDelegate();

  /*! \brief Create WLineEdit to be used in the View
   */
  std::unique_ptr<Wt::WWidget> createFormWidget() override;
};

/*! \brief %Form delegate class for std::string
 *
 * This will create a WLineEdit to display the std::string value
 * in the View
 *
 * \ingroup form
 */
template<>
class WT_API WFormDelegate<std::string, void> : public WAbstractFormDelegate
{
public:
  /*! \brief Create a form delegate
   */
  WFormDelegate();

  /*! \brief Create WLineEdit to be used in the View
   */
  std::unique_ptr<Wt::WWidget> createFormWidget() override;

  /*! \brief Update the value in the Model
   */
  void updateModelValue(Wt::WFormModel *model, Wt::WFormModel::Field field, Wt::WFormWidget *edit) override;
};

/*! \brief %Form delegate class for WDate
 *
 * This will create a WDateEdit to display the WDate value in the
 * View
 *
 * \ingroup form
 */
template<>
class WT_API WFormDelegate<Wt::WDate, void> : public WAbstractFormDelegate
{
public:
  /*! \brief Create a form delegate
   */
  WFormDelegate();

  /*! \brief Create WDateEdit to be used in the View
   */
  std::unique_ptr<Wt::WWidget> createFormWidget() override;

  /*! \brief Create WDateValidator
   */
  std::shared_ptr<Wt::WValidator> createValidator() override;

  /*! \brief Update the value in the Model
   */
  void updateModelValue(Wt::WFormModel *model, Wt::WFormModel::Field field, Wt::WFormWidget *edit) override;
};

/*! \brief %Form delegate class for WTime
 *
 * This will create a WTimeEdit to display the WTime value in the
 * View
 *
 * \ingroup form
 */
template<>
class WT_API WFormDelegate<Wt::WTime, void> : public WAbstractFormDelegate
{
public:
  /*! \brief Create a form delegate
   */
  WFormDelegate();

  /*! \brief Create WTimeEdit to be used in the View
   */
  std::unique_ptr<Wt::WWidget> createFormWidget() override;

  /*! \brief Create WTimeValidator
   */
  std::shared_ptr<Wt::WValidator> createValidator() override;

  /*! \brief Update the value in the Model
   */
  void updateModelValue(Wt::WFormModel *model, Wt::WFormModel::Field field, Wt::WFormWidget *edit) override;
};

/*! \brief %Form delegate class for WDateTime
 *
 * This will create a WLineEdit to display the WDateTime value
 * in the View
 *
 * In the future this implementation will change to return a dedicated widget
 * for WDateTime objects.
 *
 * \ingroup form
 */
template<>
class WT_API WFormDelegate<Wt::WDateTime, void> : public WAbstractFormDelegate
{
public:
  /*! \brief Create a form delegate
   */
  WFormDelegate();

  /*! \brief Create a WLineEdit to be used in the View
   */
  std::unique_ptr<Wt::WWidget> createFormWidget() override;

  /*! \brief Update the value in the Model
   */
  void updateModelValue(Wt::WFormModel *model, Wt::WFormModel::Field field, Wt::WFormWidget *edit) override;
};

/*! \brief %Form delegate class for boolean
 *
 * This will create a WCheckBox to display the boolean value
 * in the view
 *
 * \ingroup form
 */
template<>
class WT_API WFormDelegate<bool, void> : public WAbstractFormDelegate
{
public:
  /*! \brief Create a form delegate
   */
  WFormDelegate();

  /*! \brief Create WCheckBox to be used in the View
   */
  std::unique_ptr<Wt::WWidget> createFormWidget() override;

  /*! \brief Update the value in the Model
   */
  void updateModelValue(Wt::WFormModel *model, Wt::WFormModel::Field field, Wt::WFormWidget *edit) override;

  /*! \brief Update the value in the View
   */
  void updateViewValue(Wt::WFormModel *model, Wt::WFormModel::Field field, Wt::WFormWidget *edit) override;
};

/*! \brief %Form delegate class for integer
 *
 * This will create a WLineEdit to display the integer value
 * in the View. Additionally the delegate will also initialize
 * the WIntValidator for validation.
 *
 * \ingroup form
 */
template<>
class WT_API WFormDelegate<int, void> : public WAbstractFormDelegate
{
public:
  /*! \brief Create a form delegate
   */
  WFormDelegate();

  /*! \brief Create WLineEdit to be used in the View
   */
  std::unique_ptr<Wt::WWidget> createFormWidget() override;

  /*! \brief Create WIntValidator for validation
   */
  std::shared_ptr<Wt::WValidator> createValidator() override;

  /*! \brief Update the value in the Model
   */
  void updateModelValue(Wt::WFormModel *model, Wt::WFormModel::Field field, Wt::WFormWidget *edit) override;

  /*! \brief Update the value in the View
   */
  void updateViewValue(Wt::WFormModel *model, Wt::WFormModel::Field field, Wt::WFormWidget *edit) override;
};

/*! \brief %Form delegate for double
 *
 * This will create a WLineEdit to display the double value
 * in the View. Additionally the delegate will also initialize
 * the WDoubleValidator for validation.
 *
 * \ingroup form
 */
template<>
class WT_API WFormDelegate<double, void> : public WAbstractFormDelegate
{
public:
  /*! \brief Create a form delegate
   */
  WFormDelegate();

  /*! \brief Create WLineEdit to be used in the View
   */
  std::unique_ptr<Wt::WWidget> createFormWidget() override;

  /*! \brief Create WDoubleValidator for validation
   */
  std::shared_ptr<Wt::WValidator> createValidator() override;

  /*! \brief Update the value in the model
   */
  void updateModelValue(Wt::WFormModel *model, Wt::WFormModel::Field field, Wt::WFormWidget *edit) override;

  /*! \brief Update the value in the View
   */
  void updateViewValue(Wt::WFormModel *model, Wt::WFormModel::Field field, Wt::WFormWidget *edit) override;
};
  }
}

#endif // WT_FORM_FORM_DELEGATE
