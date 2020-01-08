// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2009 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "FileItem.h"

const Wt::ItemDataRole FileItem::ContentsRole = ItemDataRole::User;
const Wt::ItemDataRole FileItem::FilePathRole = ItemDataRole::User + 1;
const Wt::ItemDataRole FileItem::FileNameRole = ItemDataRole::User + 2;
