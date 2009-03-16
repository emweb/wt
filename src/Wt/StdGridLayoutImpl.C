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
#include "DomElement.h"

#ifdef WIN32
#define snprintf _snprintf
#endif
namespace Wt {

bool StdGridLayoutImpl::useJavaScriptHeights(WApplication *app)
{
  return true;

  /*
   * Who is right? According to the spec, several aspects of height
   * are not defined, and you cannot use for example a height: xx%
   * when the parents height is not explicitly defined. This makes
   * CSS-based vertical layout virtually impossible. In that respect,
   * Opera is the only compliant implementation, and sadly, it is also
   * the only thing missing to make Opera work without JavaScript.
   *
   * Of Safari and Gecko, only safari behaves well with overflowing
   * rows (to keep enforcing the table height). Firefox behaves well
   * except for when divs start overflowing. Again, this is not
   * specified in CSS.
   */

  /*
  return app->environment().agentIE()
    ||  app->environment().agentOpera()
    ||  app->environment().agentKonqueror()
    || (app->environment().agentGecko() && app->environment().javaScript());
  */
}

StdGridLayoutImpl::StdGridLayoutImpl(WLayout *layout, Impl::Grid& grid)
  : StdLayoutImpl(layout),
    grid_(grid),
    useFixedLayout_(false)
{
  const char *CSS_RULES_NAME = "StdGridLayoutImpl";

  WApplication *app = WApplication::instance();

  const bool jsHeights = useJavaScriptHeights(app);

  if (!app->styleSheet().isDefined(CSS_RULES_NAME)) {
    app->styleSheet().addRule("table.Wt-hcenter", "margin: 0px auto;",
			      CSS_RULES_NAME);

    app->doJavaScript
      (app->javaScriptClass() + ".layoutTableObjs=[];", false);

    if (jsHeights) {
      app->doJavaScript
	(WT_CLASS ".layoutAdjust=function(w,c,mh) {"
	 "" "if (" WT_CLASS ".isHidden(w))"
	 ""   "return;"
	 ""
	 "" "var WT=" WT_CLASS ","
	 ""     "t=w.firstChild;"
	 /*
	  * 'r' holds the target height for this table. If a
	  * height has been explicitly set, we use that height,
	  * otherwise we use the computed height. Note that we need to
	  * remove padding of the parent, and margin of myself.
	  */
	 "" "var r=WT.pxself(w.parentNode, 'height');"
	 "" "if (r==0) {"
	 ""   "r=w.parentNode.clientHeight;"
	 ""   "r+= -WT.px(w.parentNode, 'paddingTop')"
	 ""      "-WT.px(w.parentNode, 'paddingBottom');"
	 "" "}"
	 "" "r+= -WT.px(w, 'marginTop')"
	 ""    "-WT.px(w, 'marginBottom');"

	 /*
	  * Reduce 'r' with the total height of rows with stretch=0.
	  */
	 "" "var ts=0,"  // Sum of stretch factors
	 ""     "tmh=0," // Min heights
	 ""     "i, j, il, jl," // iterator variables
	 ""     "row, tds, td;"
	 "" "for (i=0, il=t.rows.length; i<il; i++) {"
	 ""   "tmh += mh[i];"
	 ""   "if (c[i]==0)"
	 ""     "r -= t.rows[i].offsetHeight;" // reduce r
	 ""   "else "
	 ""     "ts += c[i];"
	 "" "}"

	 "" "r=r>tmh?r:tmh;"

	 /*
	  *  Now, iterate over the whole table, and adjust the height
	  *  for every row which has a strecth and for every cell.
	  */
	 "" "if (ts!=0 && r>0) {"
	 ""   "var left=r, h;"                  // remaining space to be divided
	 ""   "for (i=0, il=t.rows.length; i<il; i++) {"
	 ""     "row=t.rows[i];"
	 ""     "if (c[i] != 0) {"

	 /*
	  *       The target height 'h', cannot be more than what is still
	  *       left to distribute, and cannot be less than the minimum
	  *       height
	  */

	 ""       "h=r*c[i]/ts;"
	 ""       "h=left>h?h:left;"
	 ""       "h=Math.round(mh[i]>h?mh[i]:h);"

	 ""       "left -= h;"
	 ""       "if (row.style.height!=h+'px'){"
	 ""         "row.style.height=h+'px';"
	 ""         "tds=row.childNodes;"

	 ""         "for (j=0, jl=tds.length; j<jl; ++j){"
	 ""           "td=tds[j];"
	 ""           "var k=h-WT.pxself(td, 'paddingTop')"
	 ""                "-WT.pxself(td, 'paddingBottom');"
	 ""           "if (k <= 0) "
	 ""             "k=0;"

	 ""           "td.style.height= k+'px';"
	 ""           "if (td.style['verticalAlign']"
	 ""               "|| td.childNodes.length == 0) continue;"
         ""           "var ch=td.childNodes[0];"   // 'ch' is cell contents
	 ""           "if (k <= 0) "
	 ""             "k=0;"

	 ""           "if (ch.className=='Wt-hcenter'){"
	 ""              "ch.style.height= k+'px';"
	 ""              "var itd=ch.firstChild.firstChild;"
	 ""              "if (itd.nodeName.toUpperCase() != 'TD')"
	 ""                "itd=itd.firstChild;"
	 ""              "if (itd.style.height!=k+'px')"
	 ""                "itd.style.height=k+'px';"
	 ""              "ch=itd.firstChild;"
	 ""           "}"

	 ""           "if (td.childNodes.length==1)"
	 ""             "k += -WT.px(ch, 'marginTop')"
	 ""                  "-WT.px(ch, 'marginBottom')"
	 ""                  "-WT.px(ch, 'borderTopWidth')"
	 ""                  "-WT.px(ch, 'borderBottomWidth')"
	 ""                  "-WT.px(ch, 'paddingTop')"
	 ""                  "-WT.px(ch, 'paddingBottom');"

	 ""           "if (k <= 0) "
	 ""             "k=0;"

	 ""           "if (ch.nodeName.toUpperCase()=='BUTTON'"
	 ""               "|| ch.nodeName.toUpperCase()=='INPUT'"
	 ""               "|| ch.nodeName.toUpperCase()=='SELECT'"
	 ""               "|| ch.nodeName.toUpperCase()=='TABLE') "
	 ""              "continue;"

	 ""           "if (ch.style.height != k+'px')"
	 ""             "ch.style.height = k+'px';"

	 ""           "if (td.childNodes.length==1"
	 ""               "&&ch.nodeName.toUpperCase()=='TEXTAREA') {"
	 + std::string(app->environment().agentOpera() ?
		       // Older opera:
		       // "ch.style.height = (k-2) + 'px';"
		       // "ch.style.marginLeft = '3px';"
		        "if (k <= 6) k=6;"
		        "ch.style.height = (k-6) + 'px';"
		        "td.style.marginLeft = '-1px';"
		       :
		        "if (k <= 8) k=8;"
		        "ch.style.height = (k-8) + 'px';"
		        "td.style.marginLeft = '-1px';"
		        "td.style.marginTop = '-1px';") +
	 ""           "}"
	 ""         "}"
         ""       "}"
	 ""     "}"
	 ""   "}"
	 "" "}"

	 /*
	  * Column widths: for every column which has no % width set,
	  * we compute the maximum width of the contents, and set this
	  * as the width of the first cell, taking into account the
	  * cell padding.
	  */
	 "" "if (t.style.tableLayout != 'fixed')"
	 ""    "return;"
	 "" "var jc=0, chn=t.childNodes;"
	 "" "for (j=0, jl=chn.length; j<jl; j++) {"
	 ""   "var col=chn[j], ch"
	 ""       "w, mw,"         // maximum column width
	 ""       "c, ci, cil;" // for finding a column
	 ""   "if (WT.hasTag(col, 'COLGROUP')) {" // IE
	 ""      "j=-1;"
	 ""      "chn=col.childNodes;"
	 ""      "jl=chn.length;"
	 ""   "}"
	 ""   "if (!WT.hasTag(col, 'COL'))"
	 ""     "continue;"
	 ""   "if (WT.pctself(col, 'width') == 0) {"
	 ""     "mw=0;"
	 ""     "for (i=0, il=t.rows.length; i<il; i++) {"
	 ""       "row=t.rows[i];"
	 ""       "tds=row.childNodes;"
	 ""       "c=0;"
	 ""       "for (ci=0, cil=tds.length; ci<cil; ci++) {"
	 ""         "td=tds[ci];"
	 ""         "if (td.colSpan==1 && c==jc && td.childNodes.length==1) {"
	 ""           "ch=td.firstChild;"
	 ""           "w=ch.offsetWidth+WT.px(ch, 'marginLeft')"
	 ""               "+WT.px(ch, 'marginRight')"
	 ""               "+WT.px(td, 'paddingLeft')"
	 ""               "+WT.px(td, 'paddingRight');"
	 ""           "mw=Math.max(mw, w);"
	 ""           "break;"
	 ""         "}"
	 ""         "c += td.colSpan;"
	 ""         "if (c>jc) "
	 ""           "break;"
	 ""       "}"
	 ""     "}"
	 ""     "if (mw>0 && WT.pxself(col, 'width') != mw)"
	 ""       "col.style.width=mw+'px';"
	 ""   "}"
	 ""   "++jc;"
	 "" "}"
	 "};", false);

      app->declareJavaScriptFunction
	("layoutsAdjust",
	 "" "function(){"
	 ""    "var a=" + app->javaScriptClass() + ".layoutTableObjs;"
	 ""    "var i;"
	 ""    "for(i=0;i<a.length;++i){"
	 ""      "var id=a[i][0];"
	 ""      "var c=a[i][1];"
	 ""      "var mh=a[i][2];"
	 ""      "var w=" WT_CLASS ".getElement(id);"
	 ""      "if(!w){"
	 ""        WT_CLASS ".arrayRemove(a, i);--i;"
	 ""      "}else{"
	 ""        WT_CLASS ".layoutAdjust(w,c,mh);"
	 ""      "}"
	 ""    "}"
	 ""  "}");

      app->addAutoJavaScript(app->javaScriptClass() + ".layoutsAdjust();");
    }
  }
}

StdGridLayoutImpl::~StdGridLayoutImpl()
{ }

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
      app->styleSheet().addRule("body, html",
				"height: 100%; width: 100%;"
				"margin: 0px; padding: 0px; border: none;");

      /*
       * If we are doing layout in the entire document, also remove the
       * automatic scrollbars.
       */
      if (app->environment().javaScript())
	if (!app->environment().agentIE6())
	  app->styleSheet().addRule("html, body", "overflow: hidden;");
	else
	  app->styleSheet().addRule("body", "overflow: hidden;");
    }

    /*
     * Normally, scrollbars are not used automatically for a container,
     * which applies to when a layout overflows.
     *
     * Only for IE we really need to set this otherwise the parent
     * increases its size automatically and then we cannot reduce in
     * size (standard behaviour is overflow visible which says the
     * parent size should not be affected). Luckily, IE does not show the
     * scrollbars unless really needed
     */
    if (app->environment().agentIE())
      container->setOverflow(WContainerWidget::OverflowAuto);
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
						int& additionalVerticalPadding,
						WApplication *app)
{
  const unsigned colCount = grid_.columns_.size();
  const unsigned rowCount = grid_.rows_.size();

  int totalColStretch = 0;
  for (unsigned col = 0; col < colCount; ++col)
    totalColStretch += grid_.columns_[col].stretch_;

  int totalRowStretch = 0;
  for (unsigned row = 0; row < rowCount; ++row)
    totalRowStretch += grid_.rows_[row].stretch_;

  int margin[] = { 0, 0, 0, 0};

  if (layout()->parentLayout() == 0)
    layout()->getContentsMargins(margin + 3, margin,
				 margin + 1, margin + 2);

  DomElement *div = DomElement::createNew(DomElement_DIV);
  div->setId(this);

  std::string divStyle;
  if (fitHeight && !app->environment().agentIE())
    divStyle += "height: 100%;";
  if (!divStyle.empty())
    div->setAttribute("style", divStyle);

  DomElement *table = DomElement::createNew(DomElement_TABLE);
  const bool jsHeights = useJavaScriptHeights(app);

  std::string style;
  if (fitWidth) {
    if (useFixedLayout_)
      style = "table-layout: fixed;";
    style += "width: 100%;";
  }
  if (!jsHeights && fitHeight)
    style += "height: 100%;";

  table->setAttribute("style", style);

  if (jsHeights) {
    std::stringstream layoutAdd;

    layoutAdd << app->javaScriptClass() << ".layoutTableObjs.push(['"
	      << div->id() << "',[";
    for (unsigned i = 0; i < rowCount; ++i) {
      if (i != 0)
	layoutAdd << ",";

      if (totalRowStretch == 0)
	layoutAdd << (fitHeight ? "1" : "0");
      else
	layoutAdd << grid_.rows_[i].stretch_;
    }

    layoutAdd << "],[";

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
    layoutAdd << "]]);";

    app->doJavaScript(layoutAdd.str());
  }

  DomElement *tbody = DomElement::createNew(DomElement_TBODY);

  if (fitWidth)
    for (unsigned col = 0; col < colCount; ++col) {
      DomElement *c = DomElement::createNew(DomElement_COL);
      int stretch = grid_.columns_[col].stretch_;
      if (stretch || (fitWidth && totalColStretch == 0)) {
	int pct = totalColStretch == 0 ? 100 / colCount
	  : 100 * stretch / totalColStretch;
	c->setAttribute
	  ("style", "width:" + boost::lexical_cast<std::string>(pct) + "%;");
      }
      table->addChild(c);
    }

  std::vector<bool> overSpanned(colCount * rowCount, false);

  for (unsigned row = 0; row < rowCount; ++row) {
    std::string heightPct;

    DomElement *tr = DomElement::createNew(DomElement_TR);

    int stretch = grid_.rows_[row].stretch_;
    if (stretch || (!jsHeights && fitHeight && totalRowStretch == 0)) {
      int pct = totalRowStretch == 0 ?
	100 / rowCount :
	100 * stretch / totalRowStretch;
      std::stringstream style;
      style << "height: " << pct << "%;";
      heightPct = style.str();
      tr->setAttribute("style", heightPct);
    }

    for (unsigned col = 0; col < colCount; ++col) {
      if (!overSpanned[row * colCount + col]) {
	Impl::Grid::Item& item = grid_.items_[row][col];

	bool itemFitWidth = (item.colSpan_ == (int)colCount)
	  || (totalColStretch == 0);
	bool itemFitHeight = (item.rowSpan_ == (int)rowCount)
	  || (totalRowStretch == 0);

	for (int i = 0; i < item.rowSpan_; ++i) {
	  if (grid_.rows_[row + i].stretch_)
	    itemFitHeight = true;
	  for (int j = 0; j < item.colSpan_; ++j) {
	    if (grid_.columns_[col + j].stretch_)
	      itemFitWidth = true;
	    if (i + j > 0)
	      overSpanned[(row + i) * colCount + col + j] = true;
	  }
	}

	// If we do not always fit heights of items (nested layouts),
	// then content of these nested layouts will not expand in
	// each cell to the full height alotted to by this grid. But
	// if we do, this makes the row no longer react to reductions
	// in height... Which is worse? I think the former?
	itemFitHeight = true;

	HorizontalAlignment hAlign
	  = (HorizontalAlignment)(item.alignment_ & 0x0F);
	VerticalAlignment vAlign
	  = (VerticalAlignment)(item.alignment_ & 0xF0);

	if (hAlign != 0)
	  itemFitWidth = false;
	if (vAlign != 0)
	  itemFitHeight = false;

	int padding[] = { 0, 0, 0, 0 };

	if (row == 0)
	  padding[0] = margin[0];
	else
	  padding[0] = (grid_.verticalSpacing_+1) / 2;

	if (row + item.rowSpan_ == rowCount)
	  padding[2] = margin[2];
	else
	  padding[2] = grid_.verticalSpacing_ / 2;

	padding[1] = padding[3] = 0;

	if (col == 0)
	  padding[3] = margin[3];
	else
	  padding[3] = (grid_.horizontalSpacing_ + 1)/2;

	if (col + item.colSpan_ == colCount)
	  padding[1] = margin[1];
	else
	  padding[1] = (grid_.horizontalSpacing_)/2;

	DomElement *td = DomElement::createNew(DomElement_TD);

	/*
	 * Needed for WTextEdit to adjust its size to the TD, since
	 * the TEXTAREA is not sized in pixels.
	 */
	if (!jsHeights)
	  td->setAttribute("class", "Wt-grtd");

	int additionalVerticalPadding = 0;

	if (item.item_) {
	  DomElement *c = getImpl(item.item_)
	    ->createDomElement(itemFitWidth, itemFitHeight,
			       additionalVerticalPadding, app);

	  if (!hAlign)
	    hAlign = AlignJustify;

	  switch (hAlign) {
	  case AlignCenter: {
	    DomElement *itable = DomElement::createNew(DomElement_TABLE);
	    itable->setAttribute("class", "Wt-hcenter");
	    if (vAlign == 0 && !jsHeights)
	      itable->setAttribute("style", "height:100%;");
	    DomElement *irow = DomElement::createNew(DomElement_TR);
	    DomElement *itd = DomElement::createNew(DomElement_TD);
	    if (!jsHeights)
	      itd->setAttribute("class", "Wt-grtd");
	    if (vAlign == 0)
	      itd->setAttribute("style", "height:100%;");
	    itd->addChild(c);
	    irow->addChild(itd);
	    itable->addChild(irow);
	    c = itable;
	    break;
	  }
	  case AlignRight:
	    c->setProperty(PropertyStyleFloat, "right");
	    break;
	  case AlignLeft:
	    c->setProperty(PropertyStyleFloat, "left");
	    break;
	  case AlignJustify:
	    if (c->getProperty(PropertyStyleWidth).empty()
		&& useFixedLayout_
		&& !app->environment().agentWebKit()
		&& !c->isDefaultInline())
	      c->setProperty(PropertyStyleWidth, "100%");
	    break;
	  default:
	    break;
	  }

	  td->addChild(c);
	}

	std::string style;

	if (!jsHeights && vAlign == 0) style += heightPct;

	style += "overflow:auto;";

	int padding2 = padding[2] + additionalVerticalPadding;

	if (padding[0] == padding[1] && padding[0] == padding2
	    && padding[0] == padding[3]) {
	  if (padding[0] != 0) {
	    char buf[100];
	    snprintf(buf, 100, "padding:%dpx;", padding[0]);
	    style += buf;
	  }
	} else {
	  char buf[100];
	  snprintf(buf, 100, "padding:%dpx %dpx %dpx %dpx;",
		   padding[0], padding[1], padding2, padding[3]);
	  style += buf;
	}

	switch (vAlign) {
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
	  td->setAttribute("style", style);

	if (item.rowSpan_ != 1)
	  td->setAttribute("rowspan",
			   boost::lexical_cast<std::string>(item.rowSpan_));
	if (item.colSpan_ != 1)
	  td->setAttribute("colspan",
			   boost::lexical_cast<std::string>(item.colSpan_));

	tr->addChild(td);
      }
    }

    tbody->addChild(tr);
  }

  table->addChild(tbody);
  div->addChild(table);

  return div;
}

}
