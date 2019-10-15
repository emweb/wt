// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef OPTIONLIST_H_
#define OPTIONLIST_H_

#include <Wt/WContainerWidget.h>

using namespace Wt;

class Option;

/**
 * @addtogroup composerexample
 */
//!@{

/*! \brief A list of options, separated by '|'
 *
 * This widget is part of the %Wt composer example.
 *
 * An OptionList displays a list of Option widgets, which are
 * separated by a '|' separator.
 *
 * For example, Foo | Bar | Huu
 *
 * When Options are hidden, the separators are adjusted so that
 * there is no separator after the last visible option. However,
 * this requires a call of update() each time an option is hidden
 * or shown. This is because the removing of separators is optimized
 * in stateless implementations, and thus in client-side JavaScript
 * code. Since the behaviour is not entirely stateless, the update()
 * method resets stateless implementations if necessary.
 *
 * \sa OptionList
 */
class OptionList : public WContainerWidget
{
public:
  /*! \brief Create an OptionList.
   */
  OptionList();

  /*! \brief Add an Option to the list.
   */
  void add(std::unique_ptr<Option> option);

  /*! \brief Updates the stateless implementations after an Option has been
   *         hidden or shown.
   */
  void update();

private:
  //! The list of options.
  std::vector<Option *> options_;

  //! The option that needs its stateless code updated.
  Option *optionNeedReset_;

  //! An option changed visibility: possibly update the separators inbetween
  void optionVisibilityChanged(Option *opt, bool hidden);

  friend class Option;
};

//!@}

#endif // OPTIONLIST_H_
