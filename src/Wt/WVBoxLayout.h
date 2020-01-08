// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WVBOX_LAYOUT_H_
#define WVBOX_LAYOUT_H_

#include <Wt/WBoxLayout.h>

namespace Wt {

/*! \class WVBoxLayout Wt/WVBoxLayout.h Wt/WVBoxLayout.h
 *  \brief A layout manager which arranges widgets vertically
 *
 * This convenience class creates a vertical box layout, laying contained
 * widgets out from top to bottom. 
 *
 * See WBoxLayout for available member methods and more information.
 *
 * \if cpp
 * Usage example:
 * \code
 * Wt::WContainerWidget *w = addWidget(std::make_unique<Wt::WContainerWidget>());
 *
 * auto layout = std::make_unique<Wt::WVBoxLayout>();
 * layout->addWidget(std::make_unique<Wt::WText>("One"));
 * layout->addWidget(std::make_unique<Wt::WText>("Two"));
 * layout->addWidget(std::make_unique<Wt::WText>("Three"));
 * layout->addWidget(std::make_unique<Wt::WText>("Four"));
 *
 * w->setLayout(std::move(layout));
 * \endcode
 * \endif
 *
 * \note First consider if you can achieve your layout using CSS !
 *
 * \sa WHBoxLayout
 */
class WT_API WVBoxLayout : public WBoxLayout
{
public:
  /*! \brief Create a new vertical box layout.
   */
  WVBoxLayout();
};

}

#endif // WVBOX_LAYOUT_H_
