// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2025 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifndef WSTACKED_VALIDATOR_H_
#define WSTACKED_VALIDATOR_H_

#include "Wt/WFormWidget.h"
#include "Wt/WValidator.h"

namespace Wt {
/*! \class WStackedValidator Wt/WStackedValidator.h Wt/WStackedValidator.h
 *  \brief A validator that combines multiple validators
 *
 * This validator allows you to combine multiple validators into a
 * single one.
 *
 * For a value to be considered valid by this validator, it must be
 * valid for all of the validators added to it. In case the value is
 * invalid for multiple validators, the validator used to generate the
 * error message is the one with the lowest index.
 */
class WT_API WStackedValidator: public WValidator
{
public:

  /*! \brief Creates an empty stacked validator.
   */
  WStackedValidator();

  //! \brief Destructor.
  virtual ~WStackedValidator();

  /*! \brief Adds a validator.
   *
   * This adds a validator at the last index if the validator is not
   * already present in the list of validators.
   */
  void addValidator(const std::shared_ptr<WValidator>& validator);

  /*! \brief Inserts a validator to the given index.
   *
   * This inserts a validator at the given index, or at last index if
   * the given index is bigger than the number of validators. Does
   * nothing if the validator is already present in the list of
   * validators.
   */
  void insertValidator(int index, const std::shared_ptr<WValidator>& validator);

  /*! \brief Removes the given validator.
   */
  void removeValidator(const std::shared_ptr<WValidator>& validator);

  /*! \brief Returns the number of validators.
   */
  int size() const { return validators_.size(); }

  /*! \brief Removes all the validators.
   */
  void clear();

  Result validate(const WT_USTRING& input) const override;

  std::string javaScriptValidate() const override;

private:
  std::vector<std::shared_ptr<WValidator> > validators_;

  void doClear();
};

  }


#endif // WSTACKED_VALIDATOR_H_