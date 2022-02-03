// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2021 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WT_FORM_DBO_FIELD_OPTIONS_H_
#define WT_FORM_DBO_FIELD_OPTIONS_H_

namespace Wt {
  namespace Form {
    namespace Dbo {
/*! \brief Configuration option for database fields
 *
 * When adding all database fields for a particular table to the model,
 * the below configuration options can be passed to the function.
 *
 * \sa FormModel<C>::addAllDboColumnsAsFields
 *
 * \ingroup form
 */
enum class FieldOptions {
  ExcludeForeignKeys = 0x1 //!< Foreign keys (pointers and collections) will be excluded
};
W_DECLARE_OPERATORS_FOR_FLAGS(FieldOptions)
    }
  }
}

#endif // WT_FORM_DBO_FIELD_OPTIONS_H_
