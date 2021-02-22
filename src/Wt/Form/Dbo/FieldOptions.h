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
 */
enum class FieldOptions {
  ExcludeForeignKeys = 0x1 // Foreign keys will be excluded
};
W_DECLARE_OPERATORS_FOR_FLAGS(FieldOptions)
    }
  }
}

#endif // WT_FORM_DBO_FIELD_OPTIONS_H_
