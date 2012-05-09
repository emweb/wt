/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER
(1, JavaScriptConstructor, "StdLayout",
 function(WT, id, fitHeight, config) {
   var self = this;
   var initialized = false;

   this.WT = WT;

   this.getId = function() {
     return id;
   };

   this.marginH = function(el) {
       var p = el.parentNode;
       var result = 0;

       if (!WT.boxSizing(el)) {
	 result = WT.px(el, 'marginLeft');
	 result += WT.px(el, 'marginRight');
	 result += WT.px(el, 'borderLeftWidth');
	 result += WT.px(el, 'borderRightWidth');
	 result += WT.px(el, 'paddingLeft');
	 result += WT.px(el, 'paddingRight');
       }

       result += WT.pxself(p, 'paddingLeft');
       result += WT.pxself(p, 'paddingRight');
       return result;
     };

   this.marginV = function(el) {
       var result = WT.px(el, 'marginTop');
       result += WT.px(el, 'marginBottom');

       /* Second condition: IE9 applys boxsizing to 'BUTTON' objects ? */
       if (!WT.boxSizing(el)
	   && !(WT.isIE && !WT.isIElt9 && WT.hasTag(el, 'BUTTON'))) {
	   result += WT.px(el, 'borderTopWidth');
	   result += WT.px(el, 'borderBottomWidth');
	   result += WT.px(el, 'paddingTop');
	   result += WT.px(el, 'paddingBottom');
       }

       return result;
     };

   this.getColumn = function(columni) {
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
   };

   this.adjustCell = function(td, height, col) {
     var shallow = height == 0;

     height -= WT.pxself(td, 'paddingTop');
     height -= WT.pxself(td, 'paddingBottom');

     if (height <= 0)
       height = 0;

     if (fitHeight)
       td.style.height = height+'px';

     if (td.style['verticalAlign'] || td.childNodes.length == 0)
       return;

     var ch = td.childNodes[0]; // 'ch' is cell contents

     if (ch.className == 'Wt-hcenter') {
       ch.style.height = height+'px';
       var itd = ch.firstChild.firstChild;
       if (!WT.hasTag(itd, 'TD'))
	 itd = itd.firstChild;
       if (itd.style.height != height+'px')
	 itd.style.height = height+'px';
       ch = itd.firstChild;
     }

     height -= this.marginV(ch);

     if (height <= 0)
       height = 0;

     if (WT.hasTag(ch, 'TABLE'))
       return;

     var p, w;

     if (!shallow) {
       p = ch.parentNode;
       w = p.offsetWidth - self.marginH(ch);

       if (p.className == 'Wt-chwrap' && w > 0) {
	 if (!WT.isIE) {
	   ch.style.position = 'relative';
	   ch = ch.firstChild;
	 }
	 ch.style.width = w + 'px';
	 ch.style.position = 'absolute';
       }
     }

     if (!shallow && ch.wtResize) {
       ch.wtResize(ch, w, height);
     } else if (ch.style.height != height+'px') {
       ch.style.height = height+'px';
       if (ch.className == 'Wt-wrapdiv') {
	 if (WT.isIE && WT.hasTag(ch.firstChild, 'TEXTAREA')) {
	   ch.firstChild.style.height
	     = (height - WT.pxself(ch, 'marginBottom')) + 'px';
	 }
       }
     }
   };

   /*
    * FIXME: we should merge getColumn() functionality here to
    * avoid repeatedly calling it
    */
   this.adjustRow = function(row, height) {
     var rowspan_tds = [];

     /*
      * We really need this, otherwise a row that contains only chwrap
      * children gets 0 height...
      */
     if (row.style.height != height + 'px')
       row.style.height = height + 'px';

     var tds = row.childNodes, j, jl, td, col;
     for (j=0, col=-1, jl = tds.length; j<jl; ++j) {
       td=tds[j];

       if (td.className != 'Wt-vrh')
	 ++col;

       if (td.rowSpan != 1) {
	 /* A shallow adjust cell, resetting height to 0 */
	 this.adjustCell(td, 0, col);
	 rowspan_tds.push({td: td, col: col});
	 continue;
       } else
	 this.adjustCell(td, height, col);
     }

     return rowspan_tds;
   };

   this.adjust = function() {
     var widget = WT.getElement(id);

     if (!widget)
       return false;

     if (self.initResize)
       self.initResize(WT, id, config);

     if (WT.isHidden(widget))
      return true;

     var t = widget.firstChild, p = widget.parentNode;

     if (t.style.height !== '')
       t.style.height = '';

     var height, heightIsParentHeight = false, thisFitHeight = fitHeight;

     if (WT.css(widget, 'position') === 'absolute')
       height = WT.pxself(widget, 'height');
     else {
       height = WT.pxself(p, 'height');
       heightIsParentHeight = true;
     }

     if (height === 0) {
       heightIsParentHeight = false;
       height = p.clientHeight;
     }

     if (!fitHeight) {
       if (t.clientHeight > height) {
	 thisFitHeight = true;
       } else {
	 height = t.clientHeight;
	 heightIsParentHeight = false;
       }
     }

     var pWidth = p.clientWidth;

     var doit = widget.dirty
	   || ((t.w !== pWidth || t.h !== height)
	       && (!thisFitHeight || (t.clientHeight != height)));

     if (!doit)
       return true;

     t.w = pWidth;
     t.h = height;

     widget.dirty = null;

     /*
      * 'r' holds the target height for this table. If a
      * height has been explicitly set, we use that height,
      * otherwise we use the computed height. Note that we need to
      * remove padding of the parent, and margin of myself.
      */
     var r = height;

     var i, il, ri, row, rowi; // iterator variables

     if (thisFitHeight) {
       if (!heightIsParentHeight) {
	 r -= WT.px(p, 'paddingTop');
	 r -= WT.px(p, 'paddingBottom');
       } else if (WT.boxSizing(p)) {
	 r -= WT.px(p, 'borderTopWidth');
	 r -= WT.px(p, 'borderBottomWidth');
	 r -= WT.px(p, 'paddingTop');
	 r -= WT.px(p, 'paddingBottom');
       }

       r -= WT.px(widget, 'marginTop');
       r -= WT.px(widget, 'marginBottom');

       /*
	* Sometimes, there may be other elements; e.g. in FIELDSET.
	* Remove the height of these too
	*/
       if (p.children && p.children.length !== 1) {
	 for (i = 0, il = p.children.length; i < il; ++i) {
	   var w = p.children[i];
	   if (w !== widget)
	     r -= $(w).outerHeight();
	 }
       }

       /*
	* Reduce 'r' with the total height of rows with stretch=0.
	*/
       var ts = 0,              // Sum of stretch factors
           tmh = 0,             // Min heights
	   j, jl, tds;          // Iterator variables

       for (i = 0, ri = 0, il = t.rows.length; i < il; i++) {
	 row = t.rows[i];

	 if (row.className) {     // Skip special rows
	   r -= row.offsetHeight; // Reduce r
	   continue;
	 }

	 tmh += config.minheight[ri];
	 if (config.stretch[ri] <= 0)
	   r -= row.offsetHeight;          // Reduce r
	 else
	   ts += config.stretch[ri];
	 ++ri;
       }

       r = r > tmh ? r : tmh;
     }

     var rowspan_tds = [];

     /*
      *  Now, iterate over the whole table, and adjust the height
      *  for every row (which has a stretch) and for every cell. Apply the
      *  same height to each cell's contents as well
      */
     var left = r, // remaining space to be divided
         stretch, mh, h;

     for (i = 0, ri = 0, il = t.rows.length; i < il; i++) {
       row = t.rows[i];

       if (row.className)
	 continue;

       stretch = config.stretch[ri];

       if (stretch == -1 || (fitHeight && stretch > 0)) {
	 /*
	  * The target height 'h', cannot be more than what is still
	  * left to distribute, and cannot be less than the minimum height
	  */

	 if (stretch !== -1) {
	   mh = config.minheight[ri];

	   h = r * stretch / ts;
	   h = left > h ? h : left;
	   h = Math.round(mh > h ? mh : h);
	   left -= h;
	 } else {
	   h = row.offsetHeight;
	 }

	 WT.addAll(rowspan_tds, this.adjustRow(row, h));
       }

       ++ri;
     }

     for (i = 0, il = rowspan_tds.length; i < il; ++i) {
       var td = rowspan_tds[i].td,
           col = rowspan_tds[i].col;
       this.adjustCell(td, td.offsetHeight, col);
     }

     /*
      * Column widths: for every column which has no % width set,
      * we compute the maximum width of the contents, and set this
      * as the width of the first cell, taking into account the
      * cell padding.
      */
     if (t.style.tableLayout !== 'fixed')
       return true;

     var jc = 0, chn = t.childNodes,
         col, td, ch,
         w, mw,      // maximum column width
	 c, ci, cil; // for finding a column
     for (j = 0, jl = chn.length; j < jl; j++) {
       col = chn[j];

       if (WT.hasTag(col, 'COLGROUP')) { // IE
	 j = -1;
	 chn = col.childNodes;
	 jl = chn.length;
       }

	if (!WT.hasTag(col, 'COL'))
	  continue;

       if (WT.pctself(col, 'width') === 0) {
	 mw = 0;
	 for (i = 0, il = t.rows.length; i < il; i++) {
	   row = t.rows[i];
	   tds = row.childNodes;
	   c = 0;
	   for (ci = 0, cil = tds.length; ci < cil; ci++) {
	     td = tds[ci];
	     if (td.colSpan === 1 && c === jc && td.childNodes.length ===1) {
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
	 if (mw > 0 && WT.pxself(col, 'width') !== mw)
	   col.style.width = mw + 'px';
       }

       ++jc;
     }

     return true;
   };

   this.contains = function(layout) {
     var thisw = WT.getElement(id);
     var otherw = WT.getElement(layout.getId());

     if (thisw && otherw)
       return WT.contains(thisw, otherw);
     else
       return false;
   };
 });

WT_DECLARE_WT_MEMBER
(2, JavaScriptPrototype, "StdLayout.prototype.initResize",
 function(WT, id, config) {
   var self = this;

   var getColumn = self.getColumn;

   function getColumnWidth(col, columni) {
     /* col.offsetWidth = 0 in webkit/chrome */
     if (col.offsetWidth > 0)
       return col.offsetWidth;
     else {
       var t = widget.firstChild, row = t.rows[0];
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

       if (!row.className) {
	 var td, j, ci, jl;
	 for (j=0, ci=0, jl=row.childNodes.length; j<jl; ++j) {
	   td = row.childNodes[j];

	   if (td.className != 'Wt-vrh') {
	     if (td.colSpan == 1 && ci == columni && td.childNodes.length > 0) {
	       var ch = td.firstChild;
	       var w = Math.max(1, width - self.marginH(ch));
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

     var rtl = $(document.body).hasClass('Wt-rtl');
     if (rtl) {
       var tmp = minDelta;
       minDelta = -maxDelta;
       maxDelta = -tmp;
     }

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

     WT.getElement(id).dirty = true;

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

     var rtl = $(document.body).hasClass('Wt-rtl');
     if (rtl)
       delta = -delta;

     if (WT.pctself(col, 'width') > 0
         && WT.pctself(coln, 'width') > 0) {
       col.style.width = '';
       adjustTo100();
     }

     if (WT.pctself(col, 'width') == 0)
       adjustColumn(ci, colw + delta);
     else
       WT.getElement(id).dirty = true;

     if (WT.pctself(coln, 'width') == 0)
       adjustColumn(ci + 1, colnw - delta);
     else
       WT.getElement(id).dirty = true;

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

     if (row.className) {
       if (row.className === 'Wt-hrh') {
	 var td = row.firstChild;
	 td.ri = ri - 1;
	 td.onmousedown = td.ontouchstart = function(event) {
	   var e = event||window.event;
	   startRowResize(this, this.ri, e);
	 };
       }
     } else {
       var td, j, ci, jl;
       for (j=0, ci=0, jl=row.childNodes.length; j<jl; ++j) {
	 td = row.childNodes[j];

	 if (td.className == 'Wt-vrh') {
	   td.ci = ci - 1;
	   td.onmousedown = td.ontouchstart = function(event) {
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

WT_DECLARE_APP_MEMBER
(1, JavaScriptObject, "layouts",
  new (function() {
    var layouts = [], adjusting = false;

    this.add = function(layout) {
      var i, il;

      for (i=0, il = layouts.length; i < il; ++i) {
        var l = layouts[i];

        if (l.getId() == layout.getId()) {
	  layouts[i] = layout;
 	  return;
        } else if (layout.contains(l)) {
	  layouts.splice(i, 0, layout);
	  return;
        }
      }
      layouts.push(layout);
    };

    this.adjust = function(id) {
      if (id) {
	var layout=$('#' + id).get(0);
	if (layout)
	  layout.dirty = true;
	return;
      }

      if (adjusting)
	return;

      adjusting = true;

      for (var i = 0; i < layouts.length; ++i) {
	var layout = layouts[i];
	if (!layout.adjust()) {
	  layout.WT.arrayRemove(layouts, i); --i;
	}
      }

      adjusting = false;
    };

  }) ()
);
