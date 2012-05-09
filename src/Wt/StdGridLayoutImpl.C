/*
 * Copyright (C) 2008 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#ifdef OLD_LAYOUT

#include <boost/lexical_cast.hpp>
#include <sstream>

#include "Wt/WApplication"
#include "Wt/WContainerWidget"
#include "Wt/WEnvironment"
#include "Wt/WGridLayout"
#include "Wt/WLogger"

#include "StdGridLayoutImpl.h"
#include "SizeHandle.h"
#include "DomElement.h"
#include "WebUtils.h"

#ifndef WT_DEBUG_JS
#include "js/StdGridLayoutImpl.min.js"
#endif

#ifdef WIN32
#define snprintf _snprintf
#endif

namespace Wt {

LOGGER("WGridLayout");

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

    app->addAutoJavaScript(app->javaScriptClass() + ".layouts.adjust();");
  }
}

bool StdGridLayoutImpl::itemResized(WLayoutItem *item)
{
  /*
   * The WT_RESIZE_JS function may have a new effect
   */
  WWidget *ww = item->widget();
  if (ww && !ww->javaScriptMember("wtResize").empty()) {
    forceUpdate_ = true;
    return true;
  }

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

void StdGridLayoutImpl::updateDom(DomElement& parent)
{
  if (forceUpdate_) {
    forceUpdate_ = false;
    WApplication *app = WApplication::instance();
    app->doJavaScript(app->javaScriptClass() + ".layouts.adjust('"
		      + id() + "');");
  }

  const unsigned colCount = grid_.columns_.size();
  const unsigned rowCount = grid_.rows_.size();

  for (unsigned i = 0; i < rowCount; ++i) {
    for (unsigned j = 0; j < colCount; ++j) {
      WLayoutItem *item = grid_.items_[i][j].item_;
      if (item) {
	WLayout *nested = item->layout();
	if (nested)
	  (dynamic_cast<StdLayoutImpl *>(nested->impl()))->updateDom(parent);
      }
    }
  }
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

void StdGridLayoutImpl::update(WLayoutItem *item)
{
  WContainerWidget *c = container();

  if (c)
    c->layoutChanged(true, false);
}

void StdGridLayoutImpl::setHint(const std::string& name,
				const std::string& value)
{
  if (name == "table-layout") {
    if (value == "fixed")
      useFixedLayout_ = true;
    else if (value == "auto")
      useFixedLayout_ = false;
    else
      LOG_ERROR("unrecognized hint value '" << value << "' for '"
		<< name << "'");
  } else
    LOG_ERROR("unrecognized hint '" << name << "'");
}

int StdGridLayoutImpl::nextRowWithItem(int row, int c) const
{
  for (row += grid_.items_[row][c].rowSpan_; row < (int)grid_.rows_.size();
       ++row) {
    for (unsigned col = 0; col < grid_.columns_.size();
	 col += grid_.items_[row][col].colSpan_) {
      if (hasItem(row, col))
	return row;
    }
  }

  return grid_.rows_.size();
}

int StdGridLayoutImpl::nextColumnWithItem(int row, int col) const
{
  for (;;) {
    col = col + grid_.items_[row][col].colSpan_;

    if (col < (int)grid_.columns_.size()) {
      for (unsigned i = 0; i < grid_.rows_.size(); ++i)
	if (hasItem(i, col))
	  return col;
    } else
      return grid_.columns_.size();
  }
}

bool StdGridLayoutImpl::hasItem(int row, int col) const
{
  WLayoutItem *item = grid_.items_[row][col].item_;

  if (item) {
    WWidget *w = item->widget();
    return !w || !w->isHidden();
  } else
    return false;
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

  if (hasResizeHandles) {
    SizeHandle::loadJavaScript(app);

    LOAD_JAVASCRIPT(app, "js/StdGridLayoutImpl.js",
		    "StdLayout.prototype.initResize", wtjs2);
  }

  int totalColStretch = 0;
  if (fitWidth)
    for (unsigned col = 0; col < colCount; ++col)
      totalColStretch += std::max(0, grid_.columns_[col].stretch_);

  int totalRowStretch = 0;
  if (fitHeight)
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

  /*
   * Perhaps we should disable height: 100% on all browsers, the spec says
   * they shouldn't react, some do something sensible, some ignore it, and
   * some react in a bad way (as if height is set to 0).
   */
  std::string divStyle;
  if (fitHeight && !app->environment().agentIsIElt(9)
      && !app->environment().agentIsWebKit())
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
	style += "table-layout: fixed;";
      style += "width: 100%;";
    }
    if (fitHeight) {
      if (!app->environment().ajax())
	style += "height: 100%;";
    }

    table->setProperty(PropertyStyle, style);
  }

  DomElement *tbody = DomElement::createNew(DomElement_TBODY);

  if (fitWidth) {
    for (unsigned col = 0; col < colCount; ++col) {
      DomElement *c = DomElement::createNew(DomElement_COL);
      int stretch = std::max(0, grid_.columns_[col].stretch_);

      if (stretch || (fitWidth && totalColStretch == 0)) {
	char buf[30];

	double pct = totalColStretch == 0 ? 100.0 / colCount
	  : (100.0 * stretch / totalColStretch);

	WStringStream ss;
	ss << "width:" << Utils::round_str(pct, 2, buf) << "%;";
	c->setProperty(PropertyStyle, ss.str());
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
  }

#ifndef WT_TARGET_JAVA
  std::vector<bool> overSpanned(colCount * rowCount, false);
#else
  std::vector<bool> overSpanned;
  overSpanned.insert(0, colCount * rowCount, false);
#endif // WT_TARGET_JAVA

  /*
   * Create grid
   */

  bool resizeHandleAbove = false;
  int prevRowWithItem = -1;

  for (unsigned row = 0; row < rowCount; ++row) {
    bool resizeHandleBelow = row < rowCount - 1
      && grid_.rows_[row].resizable_;

    DomElement *tr = DomElement::createNew(DomElement_TR);

    std::string heightPct;

    int rowStretch = std::max(0, grid_.rows_[row].stretch_);

    if (rowStretch || (fitHeight && totalRowStretch == 0)) {
      int pct = totalRowStretch == 0 ?
	100 / rowCount : 100 * rowStretch / totalRowStretch;
      std::stringstream style;
      style << "height: " << pct << "%;";
      heightPct = style.str();
      tr->setProperty(PropertyStyle, heightPct);
    }

    bool resizeHandleLeft = false;
    int prevColumnWithItem = -1;

    bool rowVisible = false;

    /*
     * Create row
     */
    for (unsigned col = 0; col < colCount; ++col) {

      bool resizeHandleRight = col < colCount - 1
	&& grid_.columns_[col - 1 + grid_.items_[row][col].colSpan_].resizable_;

      if (!overSpanned[row * colCount + col]) {
	Impl::Grid::Item& item = grid_.items_[row][col];

	/*
	 * We always fit the item to the width of the cell.
	 */
	bool itemFitWidth = true;

	/*
	 * When we are fitting height in this layout, then we are fitting
	 *  the height of this item if the item spans the entire table
	 *  height, or no row in the table has row stretch (in which case
	 *  we give each row stretch), or the item spans a row with row
	 *  stretch
	 *
	 * Unless, the item is vertically aligned to top/middle/bottom
	 */
	bool itemFitHeight
	  = fitHeight && ((item.rowSpan_ == (int)rowCount)
			  || (totalRowStretch == 0));

	int colSpan = 0;
	bool colStretch = false;

	for (int i = 0; i < item.rowSpan_; ++i) {
	  // FIXME: when spanning multiple rows, it is not clear what we
	  //        should do here. For now, let's not think of this.
	  //
	  // if stretch == -1 or >0, then we should fit height.
	  //
	  // if stretch == 0, then we should not fit height (in which
	  // case JavaScript will actively take over anyway), unless
	  // the item is a layout itself. XXX really ?

	  if (grid_.rows_[row + i].stretch_)
	    itemFitHeight = fitHeight;
	  else if (!rowStretch && !(item.item_ && item.item_->layout()))
	    itemFitHeight = false;

	  colSpan = item.colSpan_;

	  for (int j = 0; j < item.colSpan_; ++j) {
	    // there is no special meaning for column stretches
	    if (grid_.columns_[col + j].stretch_)
	      colStretch = fitWidth;

	    if (i + j > 0)
	      overSpanned[(row + i) * colCount + col + j] = true;

	    if (j + 1 < item.colSpan_ && grid_.columns_[col + j].resizable_)
	      ++colSpan;
	  }
	}

	// Use stretch = -1 to force fitting height

	AlignmentFlag hAlign = item.alignment_ & AlignHorizontalMask;
	AlignmentFlag vAlign = item.alignment_ & AlignVerticalMask;

	if (hAlign != 0 && hAlign != AlignJustify)
	  itemFitWidth = false;
	if (vAlign != 0)
	  itemFitHeight = false;

	int padding[] = { 0, 0, 0, 0 };

	bool itemVisible = hasItem(row, col);

	rowVisible = rowVisible || itemVisible;

	if (itemVisible) {
	  int nextRow = nextRowWithItem(row, col);
	  int prevRow = prevRowWithItem;

	  int nextCol = nextColumnWithItem(row, col);
	  int prevCol = prevColumnWithItem;

	  if (prevRow == -1)
	    padding[0] = margin[0];
	  else
	    if (!resizeHandleAbove)
	      padding[0] = (grid_.verticalSpacing_+1) / 2;

	  if (nextRow == (int)rowCount)
	    padding[2] = margin[2];
	  else
	    if (!resizeHandleBelow)
	      padding[2] = grid_.verticalSpacing_ / 2;

	  if (prevCol == -1)
	    padding[3] = margin[3];
	  else
	    if (!resizeHandleLeft)
	      padding[3] = (grid_.horizontalSpacing_ + 1)/2;

	  if (nextCol == (int)colCount)
	    padding[1] = margin[1];
	  else
	    if (!resizeHandleRight)
	      padding[1] = (grid_.horizontalSpacing_)/2;
	}

 	DomElement *td = DomElement::createNew(DomElement_TD);

	if (item.item_) {
	  if (app->environment().agentIsIElt(9) && vAlign == 0)
	    td->setProperty(PropertyStylePosition, "relative");

	  DomElement *c = getImpl(item.item_)
	    ->createDomElement(itemFitWidth, itemFitHeight, app);

	  if (!app->environment().agentIsIE())
	    c->setProperty(PropertyStyleBoxSizing, "border-box");

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

	    if (app->environment().agentIsIElt(9)) {
	      // IE7 and IE8 do support min-width but do not enforce it properly
	      // when in a table.
	      //  see http://stackoverflow.com/questions/2356525
	      //            /css-min-width-in-ie6-7-and-8
	      if (!c->getProperty(PropertyStyleMinWidth).empty()) {
		DomElement *spacer = DomElement::createNew(DomElement_DIV);
		spacer->setProperty(PropertyStyleWidth,
				    c->getProperty(PropertyStyleMinWidth));
		spacer->setProperty(PropertyStyleHeight, "1px");
		itd->addChild(spacer);
	      }
	    }

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

	  if (!app->environment().agentIsIE())
	    c->setProperty(PropertyStyleBoxSizing, "border-box");

	  /* Buttons are strange or I am not understanding CSS */
	  if (c->type() == DomElement_BUTTON) {
	    c->setProperty(PropertyStyleMarginLeft, "0");
	    c->setProperty(PropertyStyleMarginRight, "0");
	  }

	  /*
	   * When the cell has stretch (and we are fitting width), we
	   * use a relative/absolute pair so that we can force the
	   * cell to shrink while synchronizing its width using
	   * JavaScript.
	   *
	   * For now we do this only in rows in which we are also fitting
	   * height.
	   */

	  if (colStretch && itemFitWidth && itemFitHeight) {
	    td->setProperty(PropertyClass, "Wt-chwrap");
	    c->setProperty(PropertyStylePosition, "absolute");
	    if (!app->environment().agentIsIE()) {
	      DomElement *chwrap = DomElement::createNew(DomElement_DIV);
	      chwrap->setProperty(PropertyClass, "Wt-chwrap");
	      chwrap->addChild(c);
	      td->addChild(chwrap);
	    } else
	      td->addChild(c);
	  } else
	    td->addChild(c);

	  if (app->environment().agentIsIElt(9)) {
	    // IE7 and IE8 do support min-width but do not enforce it properly
	    // when in a table.
	    //  see http://stackoverflow.com/questions/2356525
	    //            /css-min-width-in-ie6-7-and-8
	    if (!c->getProperty(PropertyStyleMinWidth).empty()) {
	      DomElement *spacer = DomElement::createNew(DomElement_DIV);
	      spacer->setProperty(PropertyStyleWidth,
				  c->getProperty(PropertyStyleMinWidth));
	      spacer->setProperty(PropertyStyleHeight, "1px");
	      td->addChild(spacer);
	    }
	  }
	}

	{
	  WStringStream style;

	  if (vAlign == 0)
	    style << heightPct;

	  if (app->layoutDirection() == RightToLeft)
	    std::swap(padding[1], padding[3]);

	  if (padding[0] == padding[1] && padding[0] == padding[2]
	      && padding[0] == padding[3]) {
	    if (padding[0] != 0)
	      style << "padding:" << padding[0] << "px;";
	  } else
	    style << "padding:"
		  << padding[0] << "px " << padding[1] << "px "
		  << padding[2] << "px " << padding[3] << "px;";

	  if (vAlign != 0) switch (vAlign) {
	    case AlignTop:
	      style << "vertical-align:top;";
	      break;
	    case AlignMiddle:
	      style << "vertical-align:middle;";
	      break;
	    case AlignBottom:
	      style << "vertical-align:bottom;";
	    default:
	      break;
	    }

	  td->setProperty(PropertyStyle, style.str());
	}

	if (item.rowSpan_ != 1)
	  td->setProperty(PropertyRowSpan,
			  boost::lexical_cast<std::string>(item.rowSpan_));
	if (colSpan != 1)
	  td->setProperty(PropertyColSpan,
			  boost::lexical_cast<std::string>(colSpan));

	tr->addChild(td);

	if (itemVisible && resizeHandleRight) {
	  td = DomElement::createNew(DomElement_TD);
	  td->setProperty(PropertyClass, "Wt-vrh");

	  WStringStream style;
	  style << "padding:" << padding[0] << "px 0px " << padding[2] << "px;";
	  td->setProperty(PropertyStyle, style.str());

	  DomElement *div2 = DomElement::createNew(DomElement_DIV);
	  div2->setProperty(PropertyStyleWidth,
		      boost::lexical_cast<std::string>(grid_.horizontalSpacing_)
			   + "px");
	  td->addChild(div2);
	  tr->addChild(td);
	}

	if (itemVisible) {
	  resizeHandleLeft = resizeHandleRight;
	  prevColumnWithItem = col;
	}
      }
    }

    tbody->addChild(tr);

    if (rowVisible) {
      if (resizeHandleBelow) {
	tr = DomElement::createNew(DomElement_TR);
	tr->setProperty(PropertyClass, "Wt-hrh");
	std::string height
	  = boost::lexical_cast<std::string>(grid_.verticalSpacing_) + "px";
	tr->setProperty(PropertyStyleHeight, height);
	DomElement *td = DomElement::createNew(DomElement_TD);
	td->setProperty(PropertyColSpan,
			boost::lexical_cast<std::string>(colCount));
	WStringStream style2;
	style2 << "padding: 0px" << margin[1] << "px 0px" << margin[3] << "px;";
	td->setProperty(PropertyStyleHeight, style2.str());
      
	DomElement *div2 = DomElement::createNew(DomElement_DIV);
	div2->setProperty(PropertyStyleHeight, height);
	td->addChild(div2);

	tr->addChild(td);
	tbody->addChild(tr);
      }

      prevRowWithItem = row;
      resizeHandleAbove = resizeHandleBelow;
    }
  }

  table->addChild(tbody);

  std::stringstream layoutAdd;

  layoutAdd << app->javaScriptClass()
	    << ".layouts.add(new " WT_CLASS ".StdLayout( " WT_CLASS ", '"
	    << div->id() << "', "
	    << (fitHeight ? 1 : 0) << ", { stretch: [";
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

  div->addChild(table);

  return div;
}

}

#endif // OLD_LAYOUT
