// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2009 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "FileItem.h"

const int FileItem::ContentsRole = ItemDataRole::User;
const int FileItem::FilePathRole = ItemDataRole::User + 1;
const int FileItem::FileNameRole = ItemDataRole::User + 2;
