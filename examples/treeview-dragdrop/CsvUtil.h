// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef CSV_UTIL_H_
#define CSV_UTIL_H_

#include <iostream>

namespace Wt {
  class WObject;
  class WAbstractItemModel;
  class WStandardItemModel;
}

extern void readFromCsv(std::istream& f, Wt::WAbstractItemModel *model,
			int numRows = -1, bool firstLineIsHeaders = true);

extern Wt::WStandardItemModel *csvToModel(const std::string& csvFile,
					  Wt::WObject *parent,
					  bool firstLineIsHeader = true);

#endif // CSV_UTIL_H_
