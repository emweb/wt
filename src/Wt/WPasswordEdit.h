// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2025 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WPASSWORDEDIT_H_
#define WPASSWORDEDIT_H_

#include "Wt/WLineEdit.h"

namespace Wt {

/*! \class WPasswordEdit Wt/WPasswordEdit.h Wt/WPasswordEdit.h
 *  \brief A password edit.
 *
 *  A password edit is a line edit where the character are hidden.
 *
 * The widget corresponds to the HTML <tt>&lt;input type="password"&gt;</tt> tag.
 */
class WT_API WPasswordEdit : public WLineEdit
{
public:
  /*! \brief Creates a password edit with empty content.
   */
  WPasswordEdit();

  /*! \brief Creates a password edit with given content.
   */
  WPasswordEdit(const WT_USTRING& content);

protected:
  void updateDom(DomElement& element, bool all) override;

};

}

#endif // WPASSWORDEDIT_H_