// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef CSV_UTIL_H_
#define CSV_UTIL_H_

#include <iostream>

namespace Wt {
  class WAbstractItemModel;
}

/**
 * @addtogroup chartsexample
 */
/*@{*/

/*! \brief Utility function that reads a model from a CSV file.
 */
extern void readFromCsv(std::istream& f, Wt::WAbstractItemModel *model,
			int numRows = -1, bool firstLineIsHeaders = true);

/*@}*/

#endif // CSV_UTIL_H_
