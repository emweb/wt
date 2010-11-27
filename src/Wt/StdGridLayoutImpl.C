/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <boost/lexical_cast.hpp>
#include <sstream>
#include <stdio.h>

#include "Wt/WApplication"
#include "Wt/WContainerWidget"
#include "Wt/WEnvironment"
#include "Wt/WGridLayout"
#include "Wt/WLogger"

#include "StdGridLayoutImpl.h"
#include "SizeHandle.h"
#include "DomElement.h"

#include "JavaScriptLoader.h"

#ifndef WT_DEBUG_JS
#include "js/StdGridLayoutImpl.min.js"
#include "js/WtResize.min.js"
#endif

#ifdef WIN32
#define snprintf _snprintf
#endif

namespace Wt {

StdGridLayoutImpl::StdGridLayoutImpl(WLayout *layout, Impl::Grid& grid)
  : StdLayoutImpl(layout),
    grid_(grid),
    useFixedLayout_(false),
    forceUpdate_(false)
{
  const char *THIS_JS = "js/StdGridLayoutImpl.js";

  WApplication *app = WApplication::instance();

  if (!app->javaScriptLoaded(THIS_JS)) {
    app->styleSheet().addRule("table.Wt-hcenter", "margin: 0px auto;"
			      "position: relative");

    LOAD_JAVASCRIPT(app, THIS_JS, "StdLayout", wtjs1);
    LOAD_JAVASCRIPT(app, THIS_JS, "layouts", appjs1);

    app->setJavaScriptLoaded(THIS_JS);

    app->addAutoJavaScript(app->javaScriptClass() + ".layouts.adjust();");
  }
}

bool StdGridLayoutImpl::itemResized(WLayoutItem *item)
{
  /*
   * Iterate over all rows in which resized widgets (height changes) may
   * affect the layout.
   */
  const unsigned colCount = grid_.columns_.size();
  const unsigned rowCount = grid_.rows_.size();

  for (unsigned row = 0; row < rowCount; ++row)
    if (grid_.rows_[row].stretch_ <= 0) {
      for (unsigned col = 0; col < colCount; ++col)
	if (grid_.items_[row][col].item_ == item) {
	  forceUpdate_ = true;
	  return true;
	}
    }

  return false;
}

void StdGridLayoutImpl::updateDom()
{
  if (forceUpdate_) {
    forceUpdate_ = false;
    WApplication *app = WApplication::instance();
    app->doJavaScript(app->javaScriptClass() + ".layouts.adjust('"
		      + id() + "');");
  }
}

const char *StdGridLayoutImpl::childrenResizeJS()
{
  const char *THIS_JS = "js/WtResize.js";

  WApplication *app = WApplication::instance();

  if (!app->javaScriptLoaded(THIS_JS)) {
    LOAD_JAVASCRIPT(app, THIS_JS, "ChildrenResize", wtjs10);
    app->setJavaScriptLoaded(THIS_JS);
  }

  return WT_CLASS ".ChildrenResize";
}

StdGridLayoutImpl::~StdGridLayoutImpl()
{ 
  WApplication *app = WApplication::instance();

  /*
   * If it is a top-level layout (as opposed to a nested layout),
   * configure overflow of the container.
   */
  if (parentLayoutImpl() == 0) {
    if (container() == app->root()) {
      app->setBodyClass("");
      app->setHtmlClass("");
    }

    if (app->environment().agentIsIElt(9))
      container()->setOverflow(WContainerWidget::OverflowVisible);
  }
}

int StdGridLayoutImpl::minimumHeight() const
{
  const unsigned colCount = grid_.columns_.size();
  const unsigned rowCount = grid_.rows_.size();

  int total = 0;

  for (unsigned i = 0; i < rowCount; ++i) {
    int minHeight = 0;
    for (unsigned j = 0; j < colCount; ++j) {
      WLayoutItem *item = grid_.items_[i][j].item_;
      if (item)
	minHeight = std::max(minHeight, getImpl(item)->minimumHeight());
    }
    total += minHeight;
  }

  return total * (rowCount-1) * grid_.verticalSpacing_;
}

void StdGridLayoutImpl::containerAddWidgets(WContainerWidget *container)
{
  StdLayoutImpl::containerAddWidgets(container);

  if (!container)
    return;

  WApplication *app = WApplication::instance();

  /*
   * If it is a top-level layout (as opposed to a nested layout),
   * configure overflow of the container.
   */
  if (parentLayoutImpl() == 0) {
    if (container == app->root()) {
      /*
       * Reset body,html default paddings and so on if we are doing layout
       * in the entire document.
       */
      app->setBodyClass(app->bodyClass() + " Wt-layout");
      app->setHtmlClass(app->htmlClass() + " Wt-layout");
    }
  }
}

void StdGridLayoutImpl::setHint(const std::string& name,
				const std::string& value)
{
  if (name == "table-layout")
    if (value == "fixed")
      useFixedLayout_ = true;
    else if (value == "auto")
      useFixedLayout_ = false;
    else
      WApplication::instance()->log("error")
	<< "WGridLayout: unrecognized hint value '" << value
	<< "' for '" << name << "'";
  else
     WApplication::instance()->log("error")
       << "WGridLayout: unrecognized hint '" << name << "'";
}

DomElement *StdGridLayoutImpl::createDomElement(bool fitWidth, bool fitHeight,
						WApplication *app)
{
  forceUpdate_ = false;

  const unsigned colCount = grid_.columns_.size();
  const unsigned rowCount = grid_.rows_.size();

  bool hasResizeHandles = false;

  for (unsigned i = 0; i < colCount; ++i)
    if (grid_.columns_[i].resizable_) {
      hasResizeHandles = true;
      break;
    }

  if (!hasResizeHandles)
    for (unsigned i = 0; i < rowCount; ++i)
      if (grid_.rows_[i].resizable_) {
	hasResizeHandles = true;
	break;
      }

  if (hasResizeHandles
      && !app->javaScriptLoaded("js/StdGridLayoutImpl-resize.js")) {
    SizeHandle::loadJavaScript(app);

    LOAD_JAVASCRIPT(app, "js/StdGridLayoutImpl.js",
		    "StdLayout.prototype.initResize", wtjs2);

    app->setJavaScriptLoaded("js/StdGridLayoutImpl-resize.js");
  }

  int totalColStretch = 0;
  for (unsigned col = 0; col < colCount; ++col)
    totalColStretch += std::max(0, grid_.columns_[col].stretch_);

  int totalRowStretch = 0;
  for (unsigned row = 0; row < rowCount; ++row)
    totalRowStretch += std::max(0, grid_.rows_[row].stretch_);

  int margin[] = { 0, 0, 0, 0};

  if (layout()->parentLayout() == 0) {
#ifndef WT_TARGET_JAVA
    layout()->getContentsMargins(margin + 3, margin,
				 margin + 1, margin + 2);
#else // WT_TARGET_JAVA
    margin[3] = layout()->getContentsMargin(Left);
    margin[0] = layout()->getContentsMargin(Top);
    margin[1] = layout()->getContentsMargin(Right);
    margin[2] = layout()->getContentsMargin(Bottom);
#endif // WT_TARGET_JAVA
  }

  DomElement *div = DomElement::createNew(DomElement_DIV);
  div->setId(id());
  div->setProperty(PropertyStylePosition, "relative");

  std::string divStyle;
  if (fitHeight && !app->environment().agentIsIElt(9))
    divStyle += "height: 100%;";
  if (app->environment().agentIsIElt(9))
    divStyle += "zoom: 1;";
  if (!divStyle.empty())
    div->setProperty(PropertyStyle, divStyle);

  DomElement *table = DomElement::createNew(DomElement_TABLE);

  {
    std::string style;
    if (fitWidth) {
      if (useFixedLayout_)
	style = "table-layout: fixed;";
      style += "width: 100%;";
    }
    if (fitHeight) // for non-JavaScript mode
      style += "height: 100%;";

    table->setProperty(PropertyStyle, style);
  }

  DomElement *tbody = DomElement::createNew(DomElement_TBODY);

  if (fitWidth)
    for (unsigned col = 0; col < colCount; ++col) {
      DomElement *c = DomElement::createNew(DomElement_COL);
      int stretch = std::max(0, grid_.columns_[col].stretch_);

      if (stretch || (fitWidth && totalColStretch == 0)) {
	int pct = totalColStretch == 0 ? 100 / colCount
	  : 100 * stretch / totalColStretch;
	c->setProperty
	  (PropertyStyle,
	   "width:" + boost::lexical_cast<std::string>(pct) + "%;");
      }

      table->addChild(c);

      bool resizeHandleRight = col < colCount - 1
	&& grid_.columns_[col].resizable_;
      if (resizeHandleRight) {
	c = DomElement::createNew(DomElement_COL);	
	c->setProperty(PropertyStyleWidth, 
		       boost::lexical_cast<std::string>
		       (grid_.horizontalSpacing_) + "px");
	c->setProperty(PropertyClass, "Wt-vrh");
	table->addChild(c);
      }
    }

#ifndef WT_TARGET_JAVA
  std::vector<bool> overSpanned(colCount * rowCount, false);
#else
  std::vector<bool> overSpanned;
  overSpanned.insert(0, colCount * rowCount, false);
#endif // WT_TARGET_JAVA

  bool resizeHandleAbove = false;
  for (unsigned row = 0; row < rowCount; ++row) {
    bool resizeHandleBelow = row < rowCount - 1
      && grid_.rows_[row].resizable_;

    DomElement *tr = DomElement::createNew(DomElement_TR);

    std::string heightPct;
    int stretch = std::max(0, grid_.rows_[row].stretch_);
    if (stretch || (fitHeight && totalRowStretch == 0)) {
      int pct = totalRowStretch == 0 ?
	100 / rowCount :
	100 * stretch / totalRowStretch;
      std::stringstream style;
      style << "height: " << pct << "%;";
      heightPct = style.str();
      tr->setProperty(PropertyStyle, heightPct);
    }

    bool resizeHandleLeft = false;

    for (unsigned col = 0; col < colCount; ++col) {
      bool resizeHandleRight = col < colCount - 1
	&& grid_.columns_[col - 1 + grid_.items_[row][col].colSpan_].resizable_;

      if (!overSpanned[row * colCount + col]) {
	Impl::Grid::Item& item = grid_.items_[row][col];

	bool itemFitWidth = (item.colSpan_ == (int)colCount)
	  || (totalColStretch == 0);
	bool itemFitHeight = (item.rowSpan_ == (int)rowCount)
	  || (totalRowStretch == 0);

	int colSpan = 0;

	for (int i = 0; i < item.rowSpan_; ++i) {
	  // FIXME: if we span multiple rows, it is not clear what we should do ?
	  //
	  // if stretch == -1 or >0, then we should fit height
	  // if stretch == 0, then we should not fit height if no row
	  // stretch is set (in which case JavaScript will actively take
	  // over anyway)
	  if (grid_.rows_[row + i].stretch_)
	    itemFitHeight = true;
	  else if (!stretch)
	    itemFitHeight = false;

	  colSpan = item.colSpan_;
	  for (int j = 0; j < item.colSpan_; ++j) {
	    // there is no special meaning for column stretches
	    if (grid_.columns_[col + j].stretch_)
	      itemFitWidth = true;
	    if (i + j > 0)
	      overSpanned[(row + i) * colCount + col + j] = true;

	    if (j + 1 < item.colSpan_ && grid_.columns_[col + j].resizable_)
	      ++colSpan;
	  }
	}

	// If we do not always fit heights of items (nested layouts),
	// then content of these nested layouts will not expand in
	// each cell to the full height alotted to by this grid. But
	// if we do, this makes the row no longer react to reductions
	// in height... Which is worse? I think the former?
	//
	// Solved now: use stretch = -1 to force fitting height

	AlignmentFlag hAlign = item.alignment_ & AlignHorizontalMask;
	AlignmentFlag vAlign = item.alignment_ & AlignVerticalMask;

	if (hAlign != 0 && hAlign != AlignJustify)
	  itemFitWidth = false;
	if (vAlign != 0)
	  itemFitHeight = false;

	int padding[] = { 0, 0, 0, 0 };

	if (row == 0)
	  padding[0] = margin[0];
	else
	  if (!resizeHandleAbove)
	    padding[0] = (grid_.verticalSpacing_+1) / 2;

	if (row + item.rowSpan_ == rowCount)
	  padding[2] = margin[2];
	else
	  if (!resizeHandleBelow)
	    padding[2] = grid_.verticalSpacing_ / 2;

	if (col == 0)
	  padding[3] = margin[3];
	else
	  if (!resizeHandleLeft)
	    padding[3] = (grid_.horizontalSpacing_ + 1)/2;

	if (col + item.colSpan_ == colCount)
	  padding[1] = margin[1];
	else
	  if (!resizeHandleRight)
	    padding[1] = (grid_.horizontalSpacing_)/2;

 	DomElement *td = DomElement::createNew(DomElement_TD);

	if (app->environment().agentIsIElt(9))
	  td->setProperty(PropertyStylePosition, "relative");

	if (item.item_) {
	  DomElement *c = getImpl(item.item_)
	    ->createDomElement(itemFitWidth, itemFitHeight, app);

	  if (hAlign == 0)
	    hAlign = AlignJustify;

	  switch (hAlign) {
	  case AlignCenter: {
	    DomElement *itable = DomElement::createNew(DomElement_TABLE);
	    itable->setProperty(PropertyClass, "Wt-hcenter");
	    if (vAlign == 0)
	      itable->setProperty(PropertyStyle, "height:100%;");
	    DomElement *irow = DomElement::createNew(DomElement_TR);
	    DomElement *itd = DomElement::createNew(DomElement_TD);
	    if (vAlign == 0)
	      itd->setProperty(PropertyStyle, "height:100%;");
	    itd->addChild(c);
	    irow->addChild(itd);
	    itable->addChild(irow);
	    c = itable;
	    break;
	  }
	  case AlignRight:
	    if (!c->isDefaultInline())
	      c->setProperty(PropertyStyleFloat, "right");
	    else
	      td->setProperty(PropertyStyleTextAlign, "right");
	    break;
	  case AlignLeft:
	    if (!c->isDefaultInline())
	      c->setProperty(PropertyStyleFloat, "left");
	    else
	      td->setProperty(PropertyStyleTextAlign, "left");
	    break;
	  case AlignJustify:
	    if (c->getProperty(PropertyStyleWidth).empty()
		&& useFixedLayout_
		&& !app->environment().agentIsWebKit()
		&& !app->environment().agentIsGecko()
		&& !c->isDefaultInline())
	      c->setProperty(PropertyStyleWidth, "100%");
	    break;
	  default:
	    break;
	  }

	  td->addChild(c);
	}

	{
	  std::string style;

	  if (vAlign == 0) style += heightPct;

	  //style += "overflow:auto;";

	  if (padding[0] == padding[1] && padding[0] == padding[2]
	      && padding[0] == padding[3]) {
	    if (padding[0] != 0) {
#ifndef WT_TARGET_JAVA
	      char buf[100];
	      snprintf(buf, 100, "padding:%dpx;", padding[0]);
	      style += buf;
#else
	      style += "padding:"
		+ boost::lexical_cast<std::string>(padding[0]) + "px;";
#endif
	    }
	  } else {
#ifndef WT_TARGET_JAVA
	    char buf[100];
	    snprintf(buf, 100, "padding:%dpx %dpx %dpx %dpx;",
		     padding[0], padding[1], padding[2], padding[3]);
	    style += buf;
#else
	    style += "padding:"
	      + boost::lexical_cast<std::string>(padding[0]) + "px "
	      + boost::lexical_cast<std::string>(padding[1]) + "px "
	      + boost::lexical_cast<std::string>(padding[2]) + "px "
	      + boost::lexical_cast<std::string>(padding[3]) + "px;";
#endif
	  }

	  if (vAlign != 0) switch (vAlign) {
	    case AlignTop:
	      style += "vertical-align:top;";
	      break;
	    case AlignMiddle:
	      style += "vertical-align:middle;";
	      break;
	    case AlignBottom:
	      style += "vertical-align:bottom;";
	    default:
	      break;
	    }

	  if (!style.empty())
	    td->setProperty(PropertyStyle, style);
	}

	if (item.rowSpan_ != 1)
	  td->setProperty(PropertyRowSpan,
			  boost::lexical_cast<std::string>(item.rowSpan_));
	if (colSpan != 1)
	  td->setProperty(PropertyColSpan,
			  boost::lexical_cast<std::string>(colSpan));

	td->setProperty(PropertyStyleOverflowX, "hidden");

	tr->addChild(td);

	if (resizeHandleRight) {
	  td = DomElement::createNew(DomElement_TD);
	  td->setProperty(PropertyClass, "Wt-vrh");

#ifndef WT_TARGET_JAVA
	  char style[100];
	  snprintf(style, 100, "padding:%dpx 0px %dpx;", padding[0],
		   padding[2]);
#else
	  std::string style = "padding:"
	    + boost::lexical_cast<std::string>(padding[0]) + "px 0px"
	    + boost::lexical_cast<std::string>(padding[2]) + "px;";
#endif
	  td->setProperty(PropertyStyle, style);

	  DomElement *div2 = DomElement::createNew(DomElement_DIV);
	  div2->setProperty(PropertyStyleWidth,
		      boost::lexical_cast<std::string>(grid_.horizontalSpacing_)
			   + "px");
	  td->addChild(div2);
	  tr->addChild(td);
	}
      }

      resizeHandleLeft = resizeHandleRight;
    }

    tbody->addChild(tr);

    if (resizeHandleBelow) {
      tr = DomElement::createNew(DomElement_TR);
      tr->setProperty(PropertyClass, "Wt-hrh");
      std::string height
	= boost::lexical_cast<std::string>(grid_.verticalSpacing_) + "px";
      tr->setProperty(PropertyStyleHeight, height);
      DomElement *td = DomElement::createNew(DomElement_TD);
      td->setProperty(PropertyColSpan,
		      boost::lexical_cast<std::string>(colCount));
#ifndef WT_TARGET_JAVA
      char style2[100];
      snprintf(style2, 100, "padding:0px %dpx 0px %dpx;", margin[1], margin[3]);
#else
      std::string style2 = "padding: 0px"
	+ boost::lexical_cast<std::string>(margin[1]) + "px 0px"
	+ boost::lexical_cast<std::string>(margin[3]) + "px;";
#endif
      td->setProperty(PropertyStyleHeight, style2);
      
      DomElement *div2 = DomElement::createNew(DomElement_DIV);
      div2->setProperty(PropertyStyleHeight, height);
      td->addChild(div2);

      tr->addChild(td);
      tbody->addChild(tr);
    }

    resizeHandleAbove = resizeHandleBelow;
  }

  table->addChild(tbody);

  if (fitHeight) {
    std::stringstream layoutAdd;

    layoutAdd << app->javaScriptClass()
	      << ".layouts.add(new " WT_CLASS ".StdLayout( " WT_CLASS ", '"
	      << div->id() << "', { stretch: [";
    for (unsigned i = 0; i < rowCount; ++i) {
      if (i != 0)
	layoutAdd << ",";

      int stretch = 0;
      if (totalRowStretch == 0 && fitHeight)
	stretch = 1;
      else
	stretch = grid_.rows_[i].stretch_;

      layoutAdd << stretch;
    }

    layoutAdd << "], minheight: [";

    for (unsigned i = 0; i < rowCount; ++i) {
      if (i != 0)
	layoutAdd << ",";

      int minHeight = 0;

      for (unsigned j = 0; j < colCount; ++j) {
	WLayoutItem *item = grid_.items_[i][j].item_;
	if (item)
	  minHeight = std::max(minHeight, getImpl(item)->minimumHeight());
      }

      if (i == 0)
	minHeight += margin[0];
      else
	minHeight += grid_.verticalSpacing_;

      if (i == rowCount - 1)
	minHeight += margin[2];

      layoutAdd	<< minHeight;
    }
    layoutAdd << "]}));";

    table->callJavaScript(layoutAdd.str());
  }

  div->addChild(table);

  return div;
}

}
