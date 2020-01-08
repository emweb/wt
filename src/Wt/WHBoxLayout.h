// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WHBOX_LAYOUT_H_
#define WHBOX_LAYOUT_H_

#include <Wt/WBoxLayout.h>

namespace Wt {

/*! \class WHBoxLayout Wt/WHBoxLayout.h Wt/WHBoxLayout.h
 *  \brief A layout manager which arranges widgets horizontally
 *
 * This convenience class creates a horizontal box layout, laying contained
 * widgets out from left to right.
 *
 * See the WBoxLayout documentation for available member methods and
 * more information.
 *
 * Usage example:
 * \if cpp
 * \code
 * Wt::WContainerWidget *w = addWidget(std::make_unique<Wt::WContainerWidget>());
 *
 * auto layout = std::make_unique<Wt::WHBoxLayout>();
 * layout->addWidget(std::make_unique<Wt::WText>("One"));
 * layout->addWidget(std::make_unique<Wt::WText>("Two"));
 * layout->addWidget(std::make_unique<Wt::WText>("Three"));
 * layout->addWidget(std::make_unique<Wt::WText>("Four"));
 *
 * w->setLayout(std::move(layout));
 * \endcode
 * \elseif java
 * \code
 * WContainerWidget w = new WContainerWidget(this);
 *		
 * WHBoxLayout layout = new WHBoxLayout();
 * layout.addWidget(new WText("One"));
 * layout.addWidget(new WText("Two"));
 * layout.addWidget(new WText("Three"));
 * layout.addWidget(new WText("Four"));
 *	 
 * w.setLayout(layout);
 * \endcode
 * \endif
 *
 * \note First consider if you can achieve your layout using CSS !
 *
 * \sa WVBoxLayout
 */
class WT_API WHBoxLayout : public WBoxLayout
{
public:
  /*! \brief Creates a new horizontal box layout.
   */
  WHBoxLayout();
};

}

#endif // WHBOX_LAYOUT_H_
