// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef CSV_UTIL_H_
#define CSV_UTIL_H_

#include <iostream>

using namespace Wt;

namespace Wt {
  class WObject;
  class WAbstractItemModel;
  class WStandardItemModel;
}

extern void readFromCsv(std::istream& f, std::shared_ptr<WAbstractItemModel> model,
			int numRows = -1, bool firstLineIsHeaders = true);

extern std::shared_ptr<WStandardItemModel> csvToModel(const std::string& csvFile,
					  bool firstLineIsHeader = true);

#endif // CSV_UTIL_H_
