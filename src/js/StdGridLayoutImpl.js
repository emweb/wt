/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER
(1, "StdLayout",
 function(WT, id, config) {
   var self = this;
   var initialized = false;

   this.marginH = function(el) {
     var p = el.parentNode;
     return WT.px(el, 'marginLeft')
       + WT.px(el, 'marginRight')
       + WT.px(el, 'borderLeftWidth')
       + WT.px(el, 'borderRightWidth')
       + WT.px(p, 'paddingLeft')
       + WT.px(p, 'paddingRight');
   };

   this.marginV = function(el) {
     // TODO: consider caching
     return WT.px(el, 'marginTop')
       + WT.px(el, 'marginBottom')
       + WT.px(el, 'borderTopWidth')
       + WT.px(el, 'borderBottomWidth')
       + WT.px(el, 'paddingTop')
       + WT.px(el, 'paddingBottom');
   };

   this.adjustRow = function(row, height) {
     if (row.style.height != height+'px')
       row.style.height = height+'px';

     var tds = row.childNodes, j, jl, td;
     for (j=0, jl = tds.length; j<jl; ++j) {
       td=tds[j];

       var k = height - WT.pxself(td, 'paddingTop')
	 - WT.pxself(td, 'paddingBottom');
       if (k <= 0)
	 k=0;

       td.style.height = k+'px';
       if (td.style['verticalAlign'] || td.childNodes.length == 0)
	 continue;

       var ch = td.childNodes[0]; // 'ch' is cell contents
       if (k <= 0)
	 k=0;

       if (ch.className == 'Wt-hcenter') {
	 ch.style.height = k+'px';
	 var itd = ch.firstChild.firstChild;
	 if (!WT.hasTag(itd, 'TD'))
	   itd = itd.firstChild;
	 if (itd.style.height != k+'px')
	   itd.style.height = k+'px';
	 ch = itd.firstChild;
       }

       if (td.childNodes.length == 1)
	 k += -this.marginV(ch);

       if (k <= 0)
	 k=0;

       if (WT.hasTag(ch, 'TABLE'))
	 continue;

       if (ch.wtResize) {
	 var p = ch.parentNode, w = p.offsetWidth - self.marginH(ch);
	 ch.wtResize(ch, w, k);
       } else if (ch.style.height != k+'px') {
	 ch.style.height = k+'px';
	 if (ch.className == 'Wt-wrapdiv') {
	   if (WT.isIE && WT.hasTag(ch.firstChild, 'TEXTAREA')) {
	     ch.firstChild.style.height
	       = (k - WT.pxself(ch, 'marginBottom')) + 'px';
	   }
	 }
       }
     }
   };

   this.adjust = function() {
     var widget = WT.getElement(id);
     if (!widget)
       return false;

     if (self.initResize)
       self.initResize(WT, id, config);

     if (WT.isHidden(widget))
      return true;

     var t = widget.firstChild;
     if (t.style.height != '')
       t.style.height = '';

     /*
      * 'r' holds the target height for this table. If a
      * height has been explicitly set, we use that height,
      * otherwise we use the computed height. Note that we need to
      * remove padding of the parent, and margin of myself.
      */
     var r = WT.pxself(widget.parentNode, 'height');
     if (r == 0) {
       r = widget.parentNode.clientHeight;
       r += - WT.px(widget.parentNode, 'paddingTop')
	 - WT.px(widget.parentNode, 'paddingBottom');
     }

     r += - WT.px(widget, 'marginTop') - WT.px(widget, 'marginBottom');

     /*
      * Sometimes, there may be other elements; e.g. in FIELDSET.
      * Remove the height of these too
      */
     var i, il;
     if (widget.parentNode.children) {
       for (i=0, il=widget.parentNode.children.length; i<il; ++i) {
	 var w = widget.parentNode.children[i];
	   if (w != widget)
	     r -= $(w).outerHeight();
       }
     }
     /*
      * Reduce 'r' with the total height of rows with stretch=0.
      */
     var ts=0,                         // Sum of stretch factors
         tmh=0,                          // Min heights
	 ri, j, jl, row, tds, td; // Iterator variables

     for (i=0, ri=0, il=t.rows.length; i<il; i++) {
       row = t.rows[i];

       if (row.className == 'Wt-hrh') {  // Skip resize rows
	 r -= row.offsetHeight;          // Reduce r
	 td = row.firstChild;
	 continue;
       }
       tmh += config.minheight[ri];
       if (config.stretch[ri] <= 0)
	 r -= row.offsetHeight; // reduce r
       else
	 ts += config.stretch[ri];
       ++ri;
     }

     r=r>tmh?r:tmh;

     /*
      *  Now, iterate over the whole table, and adjust the height
      *  for every row (which has a stretch) and for every cell. Apply the
      *  same height to each cell's contents as well
      */
     if (ts!=0 && r>0) {
       var left=r, // remaining space to be divided
           h;

       for (i=0, ri=0, il=t.rows.length; i<il; i++) {
	 if (t.rows[i].className=='Wt-hrh') // Skip resize rows
	   continue;

	 row = t.rows[i];

	 if (config.stretch[ri] != 0) {
	   /*
	    * The target height 'h', cannot be more than what is still
	    * left to distribute, and cannot be less than the minimum
	    * height
	    */

	   if (config.stretch[ri] != -1) {
	     h=r*config.stretch[ri]/ts;
	     h=left>h?h:left;
	     h=Math.round(config.minheight[ri] > h
			  ? config.minheight[ri] : h);
	     left -= h;
	   } else {
	     h=row.offsetHeight;
	   }

	   this.adjustRow(row, h);
	 }
	 ++ri;
       }
     }

     /*
      * Column widths: for every column which has no % width set,
      * we compute the maximum width of the contents, and set this
      * as the width of the first cell, taking into account the
      * cell padding.
      */
     if (t.style.tableLayout != 'fixed')
       return true;

     var jc=0, chn=t.childNodes;
     for (j=0, jl=chn.length; j<jl; j++) {
       var col=chn[j], ch,
           w, mw,      // maximum column width
	   c, ci, cil; // for finding a column

       if (WT.hasTag(col, 'COLGROUP')) { // IE
	 j=-1;
	 chn=col.childNodes;
	 jl=chn.length;
       }

	if (!WT.hasTag(col, 'COL'))
	  continue;

       if (WT.pctself(col, 'width') == 0) {
	 mw = 0;
	 for (i=0, il=t.rows.length; i<il; i++) {
	   row = t.rows[i];
	   tds = row.childNodes;
	   c = 0;
	   for (ci=0, cil=tds.length; ci<cil; ci++) {
	     td = tds[ci];
	     if (td.colSpan==1 && c==jc && td.childNodes.length==1) {
	       ch = td.firstChild;
	       w = ch.offsetWidth + self.marginH(ch);
	       mw = Math.max(mw, w);
	       break;
	     }
	     c += td.colSpan;
	     if (c > jc)
	       break;
	    }
	 }
	 if (mw > 0 && WT.pxself(col, 'width') != mw)
	   col.style.width=mw+'px';
       }
       ++jc;
     }

     return true;
   };
 });

WT_DECLARE_WT_MEMBER
(2, "StdLayout.prototype.initResize",
 function(WT, id, config) {
   var self = this;

   function getColumn(columni) {
     var widget = WT.getElement(id),
         t = widget.firstChild;

     var i, j, jl, chn=t.childNodes;
     for (i=0, j=0, jl=chn.length; j<jl; j++) {
       var col=chn[j]; // for finding a column

       if (WT.hasTag(col, 'COLGROUP')) { // IE
	 j=-1;
	 chn=col.childNodes;
	 jl=chn.length;
       }

       if (!WT.hasTag(col, 'COL'))
	 continue;

       if (col.className != 'Wt-vrh') {
	 if (i == columni) {
	   return col;
	 } else
	   ++i;
       }
     }

     return null;
   }

   function getColumnWidth(col, columni) {
     /* col.offsetWidth = 0 in webkit/chrome */
     if (col.offsetWidth > 0)
       return col.offsetWidth;
     else {
       var t = col.parentNode, row = t.rows[0];
       var td, j, ci, jl;
       for (j=0, ci=0, jl=row.childNodes.length; j<jl; ++j) {
	 td = row.childNodes[j];

	 if (td.className != 'Wt-vrh') {
	   if (ci == columni)
	     return td.offsetWidth;
	   ci += td.colSpan;
	 }
       }
       return 0;
     }
   }

   function adjustColumn(columni, width) {
     var widget = WT.getElement(id),
         t = widget.firstChild;

     getColumn(columni).style.width = width + 'px';

     var i, ri, il, row; // Iterator variables
     for (i=0, ri=0, il=t.rows.length; i<il; i++) {
       row = t.rows[i];

       if (row.className != 'Wt-hrh') {
	 var td, j, ci, jl;
	 for (j=0, ci=0, jl=row.childNodes.length; j<jl; ++j) {
	   td = row.childNodes[j];

	   if (td.className != 'Wt-vrh') {
	     if (td.colSpan == 1 && ci == columni && td.childNodes.length==1) {
	       var ch = td.firstChild;
	       var w = width - self.marginH(ch);
	       ch.style.width = w + 'px';

	       break;
	     }

	     ci += td.colSpan;
	   }
	 }

	 ++ri;
       }
     }
   }

   function startRowResize(td, ri, event) {
     var minDelta = -td.parentNode.previousSibling.offsetHeight,
         maxDelta = td.parentNode.nextSibling.offsetHeight,
         div = td.firstChild;

     new WT.SizeHandle(WT, 'v', div.offsetHeight, div.offsetWidth,
		       minDelta, maxDelta, 'Wt-vsh',
		       function(delta) {
			 doneRowResize(td, ri, delta);
		       }, div, widget, event, 0, 0);
   }

   function startColResize(td, ci, event) {
     var minDelta = -td.previousSibling.offsetWidth,
         maxDelta = td.nextSibling.offsetWidth,
         div = td.firstChild,
         padTop = WT.pxself(t.rows[0].childNodes[0], 'paddingTop'),
	 padBottom = WT.pxself(t.rows[t.rows.length-1].childNodes[0],
			       'paddingBottom'),
	 height = t.offsetHeight - padTop - padBottom;

     new WT.SizeHandle(WT, 'h', div.offsetWidth, height,
		       minDelta, maxDelta, 'Wt-hsh',
		       function(delta) {
			 doneColResize(td, ci, delta);
		       }, div, widget, event, 0, -td.offsetTop + padTop
		       - WT.pxself(td, 'paddingTop'));
   }

   function doneRowResize(td, ri, delta) {
     var row = td.parentNode.previousSibling,
         rown = td.parentNode.nextSibling,
	 rowh = row.offsetHeight,
	 rownh = rown.offsetHeight;

     if (config.stretch[ri] > 0 && config.stretch[ri + 1] > 0)
       config.stretch[ri] = -1;

     if (config.stretch[ri + 1] == 0)
       config.stretch[ri + 1] = -1;

     if (config.stretch[ri] <= 0)
       self.adjustRow(row, rowh + delta);

     if (config.stretch[ri + 1] <= 0)
       self.adjustRow(rown, rownh - delta);

     window.onresize();
   }

   function adjustTo100() {
     var ci, totalPct = 0;

     for (ci = 0; ; ++ci) {
       var c = getColumn(ci);

       if (c)
	 totalPct += WT.pctself(c, 'width');
       else
	 break;
     }

     if (totalPct == 0)
       return;

     for (ci = 0; ; ++ci) {
       var c = getColumn(ci);

       if (c) {
	 var pct = WT.pctself(c, 'width');
	 if (pct)
	   c.style.width = (pct*100/totalPct) + '%';
       } else
	 break;
     }
   }

   function doneColResize(td, ci, delta) {
     var col = getColumn(ci),
         colw = getColumnWidth(col, ci),
	 coln = getColumn(ci + 1),
	 colnw = getColumnWidth(coln, ci + 1);

     if (WT.pctself(col, 'width') > 0
         && WT.pctself(coln, 'width') > 0) {
       col.style.width = '';
       adjustTo100();
     }

     if (WT.pctself(col, 'width') == 0)
       adjustColumn(ci, colw + delta);

     if (WT.pctself(coln, 'width') == 0)
       adjustColumn(ci + 1, colnw - delta);

     window.onresize();
   }

   var widget = WT.getElement(id);
   if (!widget)
     return;

   if (self.resizeInitialized)
     return;

   var t = widget.firstChild;

   var i, ri, il, row; // Iterator variables
   for (i=0, ri=0, il=t.rows.length; i<il; i++) {
     row = t.rows[i];

     if (row.className == 'Wt-hrh') {
       var td = row.firstChild;
       td.ri = ri - 1;
       td.onmousedown = function(event) {
	 var e = event||window.event;
	 startRowResize(this, this.ri, e);
       };
     } else {
       var td, j, ci, jl;
       for (j=0, ci=0, jl=row.childNodes.length; j<jl; ++j) {
	 td = row.childNodes[j];

	 if (td.className == 'Wt-vrh') {
	   td.ci = ci - 1;
	   td.onmousedown = function(event) {
	     var e = event||window.event;
	     startColResize(this, this.ci, e);
	   };
	 } else {
	   ci += td.colSpan;
	 }
       }

       ++ri;
     }
   }

   self.resizeInitialized = true;
 });

WT_DECLARE_APP_MEMBER(1, "layouts", []);

WT_DECLARE_APP_MEMBER
  (2, "layoutsAdjust",
   function() {
    if (this.adjusting)
      return;
    this.adjusting = true;
    var i;
    for (i=0;i < this.layouts.length; ++i) {
      var layout = this.layouts[i];
      if (!layout.adjust()) {
	this.WT.arrayRemove(this.layouts, i); --i;
      }
    }
    this.adjusting = false;
  });

