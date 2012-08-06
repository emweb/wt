/*
 * Copyright (C) 2012 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER
(1, JavaScriptConstructor, "StdLayout2",
 function(APP, id, parentId, fitWidth, fitHeight, progressive,
	  maxWidth, maxHeight, hMargins, vMargins, conf) {
   var WT = APP.WT;
   this.ancestor = null;
   this.descendants = [];

   /** @const */ var debug = false;

   /** @const */ var STRETCH = 0;
   /** @const */ var RESIZABLE = 1;
   /** @const */ var MIN_SIZE = 2;

   /** @const */ var RS_INITIAL_SIZE = 0;
   /** @const */ var RS_PCT = 1;

   /** @const */ var SPACING = 0;
   /** @const */ var MARGIN_LEFT = 1;
   /** @const */ var MARGIN_RIGHT = 2;

   /** @const */ var PREFERRED_SIZE = 0;
   /** @const */ var MINIMUM_SIZE = 1;
   /** @const */ var TOTAL_PREFERRED_SIZE = 2;
   /** @const */ var TOTAL_MINIMUM_SIZE = 3;
   /** @const */ var TOTAL_MARGIN = 4;

   /** @const */ var HORIZONTAL = 0;
   /** @const */ var VERTICAL = 1;

   /** @const */ var ALIGN_LEFT = 0x1;
   /** @const */ var ALIGN_RIGHT = 0x2;
   /** @const */ var ALIGN_CENTER = 0x4;

   /** @const */ var RESIZE_HANDLE_MARGIN = 2;
   /** @const */ var NA = 1000000;
   /** @const */ var NA_px = '-' + NA + 'px';

   var self = this;
   var config = conf;
   var itemDirty = false;  /* one or more items (with .dirty=true) need to
			      be remeasured */
   var layoutDirty = true; /* all items dirty need to be remeasured */
   var topLevel = false, parent = null, parentItemWidget = null,
     parentInitialized = false, parentMargin = [], parentMinPW = 0;

   var rtl = $(document.body).hasClass('Wt-rtl');

   var DirConfig =
     [ {
         initialized: false,
	 config: config.cols,
	 margins: hMargins,
	 maxSize: maxWidth,
	 measures: [],
	 sizes: [],
	 fixedSize: [],
	 Left: (rtl ? 'Right':'Left'),
	 left: (rtl ? 'right':'left'),
	 Right: (rtl ? 'Left':'Right'),
	 Size: 'Width',
	 size: 'width',
	 alignBits: 0,
	 getItem: function (ci, ri) {
	   return config.items[ri * DirConfig[HORIZONTAL].config.length + ci];
	 },
	 handleClass: 'Wt-vrh2',
	 resizeDir: 'h',
	 resizerClass: 'Wt-hsh2',
	 fitSize: fitWidth
       }, {
         initialized: false,
	 config: config.rows,
	 margins: vMargins,
	 maxSize: maxHeight,
	 measures: [],
	 sizes: [],
	 fixedSize: [],
	 Left: 'Top',
	 left: 'top',
	 Right: 'Bottom',
	 Size: 'Height',
	 size: 'height',
	 alignBits: 4,
	 getItem: function (ri, ci) {
	   return config.items[ri * DirConfig[HORIZONTAL].config.length + ci];
	 },
	 handleClass: 'Wt-hrh2',
	 resizeDir: 'v',
	 resizerClass: 'Wt-vsh2',
	 fitSize: fitHeight
       }];

   jQuery.data(document.getElementById(id), 'layout', this);

   function calcPreferredSize(element, dir) {
     var DC = DirConfig[dir];
     var scrollWidth = dir ? element.scrollHeight : element.scrollWidth;
     var clientWidth = dir ? element.clientHeight : element.clientWidth;
     var offsetWidth = dir ? element.offsetHeight : element.offsetWidth;

     /*
      * Firefox adds the -NA offset to the reported width ??
      */
     if (clientWidth >= NA) {
       clientWidth -= NA;
       if (scrollWidth >= NA)
	 scrollWidth -= NA;
     }

     if (scrollWidth === 0) {
       scrollWidth = WT.pxself(element, DC.size);
     }

     if (scrollWidth === clientWidth) {
       // might be too big, investigate children
       // TODO
     }

     if (WT.isIE &&
	 (WT.hasTag(element, 'BUTTON') || WT.hasTag(element, 'TEXTAREA')
	  || WT.hasTag(element, 'INPUT') || WT.hasTag(element, 'SELECT'))) {
       scrollWidth = clientWidth;
     }

     if (!WT.isOpera && !WT.isGecko)
       scrollWidth +=
	 WT.px(element, 'border' + DC.Left + 'Width') +
	 WT.px(element, 'border' + DC.Right + 'Width');

     scrollWidth +=
       WT.px(element, 'margin' + DC.Left) +
       WT.px(element, 'margin' + DC.Right);

     if (!WT.boxSizing(element) && !WT.isIE)
       scrollWidth +=
	 WT.px(element, 'padding' + DC.Left) +
	 WT.px(element, 'padding' + DC.Right);

     if (WT.isWebKit && scrollWidth > offsetWidth) {
       var i, il;
       for (i = 0, il = element.childNodes.length; i < il; ++i) {
	 var c = element.childNodes[i];
	 if (c.nodeType == 1) {
	   var z = c.style['zIndex'];
	   if (z > 0) {
	     scrollWidth = 0;
	     break;
	   }
	 }
       }
     }

     /* Must be because of a scrollbar */
     if (scrollWidth < offsetWidth)
       scrollWidth = offsetWidth;

     return scrollWidth;
   }

   function calcMinimumSize(element, dir) {
     var DC = DirConfig[dir];

     if (element.style.display == 'none')
       return 0;
     else {
       if (element['layoutMin' + DC.Size])
	 return element['layoutMin' + DC.Size];
       else {
	 var result = WT.px(element, 'min' + DC.Size);

	 if (!WT.boxSizing(element))
	   result += WT.px(element, 'padding' + DC.Left) +
	     WT.px(element, 'padding' + DC.Right);

	 return result;
       }
     }
   }

   function margin(el, dir) {
     var DC = DirConfig[dir];

     var result = WT.px(el, 'margin' + DC.Left)
       + WT.px(el, 'margin' + DC.Right);

     /* Second condition: IE9 applys boxsizing to 'BUTTON' objects ? */
     if (!WT.boxSizing(el)
	 && !(WT.isIE && !WT.isIElt9 && WT.hasTag(el, 'BUTTON'))) {
       result += WT.px(el, 'border' + DC.Left + 'Width') +
	   WT.px(el, 'border' + DC.Right + 'Width') +
	   WT.px(el, 'padding' + DC.Left) +
	   WT.px(el, 'padding' + DC.Right);
     }

     return result;
   }

   /*
    * Returns padding: different between contents size and clientSize
    */
   function padding(el, dir) {
     var DC = DirConfig[dir];

     return WT.px(el, 'padding' + DC.Left) + WT.px(el, 'padding' + DC.Right);
   }

   /*
    * Returns padding: different between contents size and reported 'size'
    */
   function sizePadding(el, dir) {
     if (WT.boxSizing(el)) {
       var DC = DirConfig[dir];

       return WT.px(el, 'border' + DC.Left + 'Width') +
	   WT.px(el, 'border' + DC.Right + 'Width') +
	   WT.px(el, 'padding' + DC.Left) +
	   WT.px(el, 'padding' + DC.Right);
     } else
       return 0;
   }

   /*
    * Returns all-inclusive margin, border and padding
    */
   function boxMargin(el, dir) {
     var DC = DirConfig[dir];

     return WT.px(el, 'border' + DC.Left + 'Width') +
       WT.px(el, 'border' + DC.Right + 'Width') +
       WT.px(el, 'margin' + DC.Left) +
       WT.px(el, 'margin' + DC.Right) +
       WT.px(el, 'padding' + DC.Left) +
       WT.px(el, 'padding' + DC.Right);
   }

   function setItemDirty(item, scheduleAdjust) {
     item.dirty = true;
     itemDirty = true;
     if (scheduleAdjust)
       APP.layouts2.scheduleAdjust();
   };

   function setCss(widget, property, value) {
     if (widget.style[property] !== value) {
       widget.style[property] = value;
       return true;
     } else
       return false;
   }

   function measure(dir, widget, container)
   {
     var DC = DirConfig[dir],
         OC = DirConfig[dir ^ 1],
         measures = DC.measures,
         dirCount = DC.config.length,
         otherCount = OC.config.length,
         maxSize = DC.maxSize;

     if (!itemDirty && !layoutDirty)
       return;

     if (container && typeof DC.minSize == 'undefined') {
       DC.minSize = WT.px(container, 'min' + DC.Size);
       if (DC.minSize > 0)
	 DC.minSize -= sizePadding(container, dir);
     }

     var prevMeasures = measures.slice();
     if (prevMeasures.length == 5) {
       prevMeasures[PREFERRED_SIZE] = prevMeasures[PREFERRED_SIZE].slice();
       prevMeasures[MINIMUM_SIZE] = prevMeasures[MINIMUM_SIZE].slice();
     }

     var preferredSize = [], minimumSize = [],
       totalPreferredSize = 0, totalMinSize = 0, di, oi;

     var measurePreferredForStretching = true;

     for (di = 0; di < dirCount; ++di) {
       var dPreferred = 0;
       var dMinimum = DC.config[di][MIN_SIZE];
       var allHidden = true;

       for (oi = 0; oi < otherCount; ++oi) {
	 var item = DC.getItem(di, oi);
	 if (item) {
	   if (!item.w || (dir == HORIZONTAL && item.dirty)) {
	     var $w = $("#" + item.id);
	     var w2 = $w.get(0);
	     if (w2 != item.w) {
	       item.w = w2;
	       (function() { var citem = item;
		 $w.find("img").load(function() { setItemDirty(citem); });
	       })();

	       item.w.style[DC.left] = item.w.style[OC.left] = NA_px;
	     }
	   }

	   if (!progressive && item.w.style.position != 'absolute') {
	     item.w.style.position = 'absolute';
	     item.w.style.visibility = 'hidden';

	     if (!item.w.wtResize) {
	       item.w.style.boxSizing = 'border-box';
	       var cssPrefix = WT.cssPrefix('BoxSizing');
	       if (cssPrefix)
		 item.w.style[cssPrefix + 'BoxSizing'] = 'border-box';
	     }
	   }

	   if (!item.ps)
	     item.ps = []; // preferred size

	   if (!item.ms)
	     item.ms = []; // minimum size

	   if (!item.size)
	     item.size = []; // set size

	   if (!item.psize)
	     item.psize = []; // set size (incl. margins, like preferred size)

	   if (!item.fs)
	     item.fs = []; // fixed size (size defined by inline size or CSS)

	   if (!item.set)
	     item.set = [false, false];

	   if (item.w) {
	     if (WT.isIE)
	       item.w.style.visibility = '';

	     if (debug)
	       console.log("measure " + dir + " "
	 		   + item.id + ': ' + item.ps[0] + ',' + item.ps[1]);

	     if (item.dirty || layoutDirty) {
	       var wMinimum = calcMinimumSize(item.w, dir);
	       if (wMinimum > dMinimum)
		 dMinimum = wMinimum;
	       item.ms[dir] = wMinimum;

	       /*
		* if we do not have an size set, we can and should take into
		* account the size set for a widget by CSS. But we can't really
		* read this -- computedStyle for width or height measures
		* instead of interpreting the stylesheet ... !
		*/
               if (!item.set[dir]) {
		 if (dir == HORIZONTAL) {
		   var fw = WT.pxself(item.w, DC.size);
		   if (fw)
		     item.fs[dir] = fw + margin(item.w, dir);
		   else
		     item.fs[dir] = 0;
		 } else {
		   var fw = WT.px(item.w, DC.size);
		   if (fw > Math.max(sizePadding(item.w, dir), wMinimum))
		     item.fs[dir] = fw + margin(item.w, dir);
		   else
		     item.fs[dir] = 0;
		 }
   	       }

	       var alignment = (item.align >> DC.alignBits) & 0xF;
	       var wPreferred = item.fs[dir];

	       if (alignment
		   || measurePreferredForStretching
		   || (DC.config[di][STRETCH] <= 0)) {

		 if (!item.layout) {
		   var calculated = calcPreferredSize(item.w, dir);

		   /*
		    * If we've set the size then we should not take the
		    * set size as the preferred size, instead we revert
		    * to a previous preferred size.
		    */
		   var sizeSet = item.set[dir] && 
		     (calculated === item.psize[dir]);

		   /*
		    * If this is an item that is stretching and has
		    * been stretched (item.set[dir]), then we should
		    * not remeasure the preferred size since it might
		    * confuse the user with constant resizing.
		    */
		   var stretching = (typeof item.ps[dir] !== 'undefined')
		     && (DC.config[di][STRETCH] > 0)
		     && item.set[dir];
		   
		   if (sizeSet || stretching)
		     wPreferred = Math.max(wPreferred, item.ps[dir]);
		   else
		     wPreferred = Math.max(wPreferred, calculated);

		   item.ps[dir] = wPreferred;
		 } else {
		   wPreferred = Math.max(wPreferred, item.ps[dir]);
		   item.ps[dir] = wPreferred;
		 }
	       } else if (item.layout)
		 wPreferred = Math.max(wPreferred, item.ps[dir]);

	       if (!item.span || item.span[dir] == 1)
		 if (wPreferred > dPreferred)
		   dPreferred = wPreferred;
	     } else {
	       if (!item.span || item.span[dir] == 1) {
		 if (item.ps[dir] > dPreferred)
		   dPreferred = item.ps[dir];
		 if (item.ms[dir] > dMinimum)
		   dMinimum = item.ms[dir];
	       }
	     }

	     if (debug)
	       console.log(" ->" + item.id + ': ' + item.ps[0] + ","
			   + item.ps[1]);

	     // XXX second condition is a hack for WTextEdit
	     if (item.w.style.display !== 'none'
		 || (WT.hasTag(item.w, 'TEXTAREA') && item.w.wtResize))
	       allHidden = false;
	   }
	 }
       }

       if (!allHidden) {
	 if (dMinimum > dPreferred)
	   dPreferred = dMinimum;
       } else {
	 // make minimum width/height consistent with preferred
	 dMinimum = dPreferred = -1;
       }

       preferredSize[di] = dPreferred;
       minimumSize[di] = dMinimum;

       if (dMinimum > -1) {
	 totalPreferredSize += dPreferred;
	 totalMinSize += dMinimum;
       }
     }

     var totalMargin = 0, first = true, rh = false;
     for (di = 0; di < dirCount; ++di) {
       if (minimumSize[di] > -1) {
	 if (first) {
	   totalMargin += DC.margins[MARGIN_LEFT];
	   first = false;
	 } else {
	   totalMargin += DC.margins[SPACING];
	   if (rh)
	     totalMargin += RESIZE_HANDLE_MARGIN * 2;
	 }

	 rh = DC.config[di][RESIZABLE] !== 0;
       }
     }

     if (!first)
       totalMargin += DC.margins[MARGIN_RIGHT];

     totalPreferredSize += totalMargin;
     totalMinSize += totalMargin;

     if (debug)
       console.log("measured " + id + ': ' + dir + " ps " + preferredSize);

     DC.measures = [
	     preferredSize,
	     minimumSize,
	     totalPreferredSize,
	     totalMinSize,
	     totalMargin
	     ];

     /*
      * If we are directly in a parent layout, then we want to
      * mark the corresponding cell as dirty if the TOTAL_PREFERRED_SIZE
      * has changed.
      */
     if (parent) {
       if (prevMeasures[TOTAL_PREFERRED_SIZE]
	   != DC.measures[TOTAL_PREFERRED_SIZE]) {
	 var totalPs = DC.measures[TOTAL_PREFERRED_SIZE];

	 if (dir == HORIZONTAL)
	   totalPs = Math.max(totalPs, parentMinPW);

	 totalPs += parentMargin[dir];

	 parent.setChildSize(parentItemWidget, dir, totalPs);
       }
     }

     /*
      * If our minimum layout requirements have changed, then we want
      * to communicate this up using the minimum widths
      *  -- FIXME IE6
      */
     if (container
	 && DC.minSize == 0
	 && prevMeasures[TOTAL_MINIMUM_SIZE] != DC.measures[TOTAL_MINIMUM_SIZE]
	 && container.parentNode.className != 'Wt-domRoot') {
       var w = DC.measures[TOTAL_MINIMUM_SIZE] + 'px';
       if (setCss(container, 'min' + DC.Size, w))
	 if (self.ancestor)
	   self.ancestor.setContentsDirty(container);
     }

     if (container) {
       if (dir == HORIZONTAL && container && WT.hasTag(container, "TD")) {
	 /*
	  * A table will otherwise not provide any room for this 0-width cell
	  */
	 setCss(container, DC.size, DC.measures[TOTAL_PREFERRED_SIZE] + 'px');
       }
     }
   }

   function finishResize(dir, di, delta) {
     var DC = DirConfig[dir];

     if (rtl) delta = -delta;

     /*
      * If di is stretchable and di + 1 isn't, then fix the size of
      * di+1 (this seems to match with user intuition after around
      * midnight).
      */
     if (DC.config[di][STRETCH] > 0 && DC.config[di + 1][STRETCH] == 0) {
       ++di;
       delta = -delta;
     }

     DC.fixedSize[di] = DC.sizes[di] + delta;

     APP.layouts2.scheduleAdjust();
   }

   function startResize(dir, handle, event) {
     var di = handle.di, DC = DirConfig[dir], OC = DirConfig[dir ^ 1],
       minDelta, maxDelta,
       widget = WT.getElement(id);

     var ri;
     for (ri = di - 1; ri >= 0; --ri) {
       if (DC.sizes[ri] >= 0) {
	 minDelta = -(DC.sizes[ri] - DC.measures[MINIMUM_SIZE][ri]);
	 break;
       }
     }

     maxDelta = DC.sizes[di] - DC.measures[MINIMUM_SIZE][di];

     if (rtl) {
       var t = minDelta;
       minDelta = -maxDelta;
       maxDelta = -t;
     }

     new WT.SizeHandle(WT, DC.resizeDir, WT.pxself(handle, DC.size),
		       WT.pxself(handle, OC.size), minDelta, maxDelta,
		       DC.resizerClass, function(delta) {
			 finishResize(dir, ri, delta);
		       }, handle, widget, event, 0, 0);
   }

   function apply(dir, widget) {
     var DC = DirConfig[dir],
       OC = DirConfig[dir ^ 1],
       measures = DC.measures,
       cSize = 0, cClientSize = false, cPaddedSize = false, noStretch = false;

     var container = topLevel ? widget.parentNode : null,
       containerParent = topLevel ? container.parentNode : null;

     if (DC.maxSize === 0) {
       if (container) {
	 var pc = WT.css(container, 'position');

	 if (pc === 'absolute')
	   cSize = WT.pxself(container, DC.size);

	 if (cSize === 0) {
	   if (!DC.initialized) {

	     if (pc !== 'absolute') {
	       cSize = dir ? container.clientHeight : container.clientWidth;

	       cClientSize = true;

	       if (dir == 0 && cSize == 0 && WT.isIElt9) {
		 cSize = container.offsetWidth;
		 cClientSize = false;
	       }

	       /*
		* heuristic to switch to layout-sizes-container mode
		*/
	       var minSize, ieCSize;

	       if ((WT.hasTag(container, "TD") || WT.hasTag(container, "TH"))
		   && !(WT.isIE && !WT.isIElt9)) {
		 minSize = 0;
		 ieCSize = 1;
	       } else {
		 if (DC.minSize) // original minSize
		   minSize = DC.minSize;
		 else            // set minSize
		   minSize = measures[TOTAL_MINIMUM_SIZE];
		 ieCSize = 0;
	       }

	       if ((WT.isIElt9 && cSize == ieCSize)
		   || (cSize == minSize + padding(container, dir))) {
		 if (debug)
		   console.log('switching to managed container size '
			       + dir + ' ' + id);
		 DC.maxSize = 999999;
	       }
	     }
	   }

	   if (cSize === 0 && DC.maxSize === 0) {
	     cSize = dir ? container.clientHeight : container.clientWidth;
	     cClientSize = true;
	   }
	 }
       } else {
	 cSize = WT.pxself(widget, DC.size);
	 cPaddedSize = true;
       }
     }

     var otherPadding = 0;
     if (container
	 && dir == VERTICAL
	 && WT.hasTag(container, 'FIELDSET')
	 && container.children.length == 2) {
       otherPadding = container.firstChild.offsetHeight;
     }

     var totalPreferredSize = measures[TOTAL_PREFERRED_SIZE];
     if (totalPreferredSize < DC.minSize)
       totalPreferredSize = DC.minSize;

     if (DC.maxSize) {
       // (2) adjust container width/height
       if (totalPreferredSize + otherPadding < DC.maxSize) {
	 setCss(container, DC.size,
		(totalPreferredSize
		 + otherPadding + sizePadding(container, dir)) + 'px');
	 cSize = totalPreferredSize + otherPadding;
	 cPaddedSize = true;
	 noStretch = true;
       } else {
	 cSize = DC.maxSize;
	 cClientSize = false;
       }
     }

     DC.cSize = cSize;

     if (dir == VERTICAL && container && container.wtResize) {
       var w = OC.cSize,
	   h = DC.cSize;
       container.wtResize(container, Math.round(w), Math.round(h));
     }

     cSize -= otherPadding;

     if (!cPaddedSize) {
       var p = 0;

       if (typeof DC.cPadding === 'undefined') {
	 if (cClientSize)
	   p = padding(container, dir);
	 else
	   p = sizePadding(container, dir);

	 DC.cPadding = p;
       } else
	 p = DC.cPadding;

       cSize -= p;
     }

     DC.initialized = true;

     if (debug)
       console.log("apply " + id + ': '
		   + dir + " ps " + measures[PREFERRED_SIZE]
		   + " cSize " + cSize);

     if (container && cSize <= 0)
       return;

     // (2a) if we can't satisfy minimum sizes, then overflow container
     if (cSize < measures[TOTAL_MINIMUM_SIZE] - otherPadding)
       cSize = measures[TOTAL_MINIMUM_SIZE] - otherPadding;

     $(widget).children("." + OC.handleClass)
       .css(DC.size,
	    (cSize - DC.margins[MARGIN_RIGHT] - DC.margins[MARGIN_LEFT])
	    + 'px');

     // (3) compute column/row widths
     var targetSize = [], dirCount = DC.config.length,
       otherCount = OC.config.length;

     if (cSize >= measures[TOTAL_MINIMUM_SIZE] - otherPadding) {
       // (1) fixed size columns/rows get their fixed width
       // (2) other colums/rows:
       //   if all columns are not stretchable: make them all stretchable
       //
       //   can non-stretchable ones get their preferred size ?
       //      (assuming stretchables take minimum size)
       //    -> yes: do so;
       //       can stretchable ones get their preferred size ?
       //       -> yes; do so and distribute excess
       //       -> no: shrink according to their preferred size
       //              - min size difference
       //    -> no: shrink non-stretchable ones according to their preferred
       //           size - min size difference
       //       set stretchable ones to minimum size

       /** @const */ var FIXED_SIZE = -1;
       /** @const */ var HIDDEN = -2;
       /** @const */ var NON_STRETCHABLES = 0;
       /** @const */ var STRETCHABLES = 1;

       var toDistribute = cSize - measures[TOTAL_MARGIN];

       var stretch = [];
       var totalMinimum = [0, 0], totalPreferred = [0, 0],
	 totalStretch = 0;

       for (var di = 0; di < dirCount; ++di) {
	 if (measures[MINIMUM_SIZE][di] > -1) {

	   var fs = -1;
	   if (typeof DC.fixedSize[di] !== "undefined")
	     fs = DC.fixedSize[di];
	   else if ((DC.config[di][RESIZABLE] !== 0)
		    && (DC.config[di][RESIZABLE][RS_INITIAL_SIZE] >= 0)) {
	     fs = DC.config[di][RESIZABLE][RS_INITIAL_SIZE];
	     if (DC.config[di][RESIZABLE][RS_PCT])
	       fs = (cSize - measures[TOTAL_MARGIN]) * fs / 100;
	   }

	   if (fs >= 0) {
	     stretch[di] = FIXED_SIZE;
	     targetSize[di] = fs;
	     toDistribute -= targetSize[di];
	   } else {
	     var category;
	     if (DC.config[di][STRETCH] > 0) {
	       category = STRETCHABLES;
	       stretch[di] = DC.config[di][STRETCH];
	       totalStretch += stretch[di];
	     } else {
	       category = NON_STRETCHABLES;
	       stretch[di] = 0;
	     }

	     totalMinimum[category] += measures[MINIMUM_SIZE][di];
	     totalPreferred[category] += measures[PREFERRED_SIZE][di];

	     targetSize[di] = measures[PREFERRED_SIZE][di];
	   }
	 } else
	   stretch[di] = HIDDEN;
       }

       if (totalStretch == 0) {
	 for (di = 0; di < dirCount; ++di)
	   if (stretch[di] == 0) {
	     stretch[di] = 1;
	     ++totalStretch;
	   }

	 totalPreferred[STRETCHABLES] = totalPreferred[NON_STRETCHABLES];
	 totalMinimum[STRETCHABLES] = totalMinimum[NON_STRETCHABLES];

	 totalPreferred[NON_STRETCHABLES] = 0;
	 totalMinimum[NON_STRETCHABLES] = 0;
       }

       if (toDistribute >
	   totalPreferred[NON_STRETCHABLES] + totalMinimum[STRETCHABLES]) {
	 toDistribute -= totalPreferred[NON_STRETCHABLES];

	 if (toDistribute > totalPreferred[STRETCHABLES]) {
	   if (DC.fitSize) {
	     // enlarge stretchables according to their stretch factor
	     toDistribute -= totalPreferred[STRETCHABLES];

	     var factor = toDistribute / totalStretch;

	     for (di = 0; di < dirCount; ++di) {
	       if (stretch[di] > 0)
		 targetSize[di] += Math.round(stretch[di] * factor);
	     }
	   }
	 } else {
	   // shrink stretchables up to their minimum size

	   var category = STRETCHABLES;

	   if (toDistribute < totalMinimum[category])
	     toDistribute = totalMinimum[category];

	   var factor;

	   if (totalPreferred[category] - totalMinimum[category] > 0)
	     factor = (toDistribute - totalMinimum[category])
	       / (totalPreferred[category] - totalMinimum[category]);
	   else
	     factor = 0;

	   for (di = 0; di < dirCount; ++di) {
	     if (stretch[di] > 0) {
	       var s = measures[PREFERRED_SIZE][di]
		 - measures[MINIMUM_SIZE][di];
	       targetSize[di] = measures[MINIMUM_SIZE][di]
		 + Math.round(s * factor);
	     }
	   }
	 }
       } else {
	 for (di = 0; di < dirCount; ++di)
	   if (stretch[di] > 0)
	     targetSize[di] = measures[MINIMUM_SIZE][di];
	 toDistribute -= totalMinimum[STRETCHABLES];

	 // shrink non-stretchables up to their minimum size
	 var category = NON_STRETCHABLES;

	 if (toDistribute < totalMinimum[category])
	   toDistribute = totalMinimum[category];

	 var factor;

	 if (totalPreferred[category] - totalMinimum[category] > 0)
	   factor = (toDistribute - totalMinimum[category])
	     / (totalPreferred[category] - totalMinimum[category]);
	 else
	   factor = 0;

	 for (di = 0; di < dirCount; ++di) {
	   if (stretch[di] == 0) {
	     var s = measures[PREFERRED_SIZE][di]
	       - measures[MINIMUM_SIZE][di];
	     targetSize[di] = measures[MINIMUM_SIZE][di]
	       + Math.round(s * factor);
	   }
	 }
       }
     } else
       targetSize = measures[MINIMUM_SIZE];

     DC.sizes = targetSize;

     if (debug)
       console.log(" -> targetSize: " + targetSize);

     // (4) set widths/heights of cells
     var left = 0, first = true, resizeHandle = false, oi, di;

     for (di = 0; di < dirCount; ++di) {
       if (targetSize[di] > -1) {
	 var thisResized = resizeHandle;
	 if (resizeHandle) {
	   var hid = id + "-rs" + dir + "-" + di;
	   var handle = WT.getElement(hid);
	   if (!handle) {
	     handle = document.createElement('div');
	     handle.setAttribute('id', hid);
	     handle.di = di;
	     handle.style.position='absolute';
	     handle.style[OC.left] = OC.margins[MARGIN_LEFT] + 'px';
	     handle.style[DC.size] = DC.margins[SPACING] + 'px';
	     handle.className = DC.handleClass;
	     widget.insertBefore(handle, widget.firstChild);

	     handle.onmousedown = handle.ontouchstart = function(event) {
	       var e = event||window.event;
	       startResize(dir, this, e);
	     };
	   }

	   left += RESIZE_HANDLE_MARGIN;
	   setCss(handle, DC.left, left + 'px');
	   left += RESIZE_HANDLE_MARGIN;
	 }

	 resizeHandle = DC.config[di][RESIZABLE] !== 0;

	 if (first) {
	   left += DC.margins[MARGIN_LEFT];
	   first = false;
	 } else
	   left += DC.margins[SPACING];

	 for (oi = 0; oi < otherCount; ++oi) {
	   var item = DC.getItem(di, oi);
	   if (item && item.w) {
	     var w = item.w;

	     var ts = targetSize[di];
	     if (item.span) {
	       var si;

	       var rs = resizeHandle;
	       for (si = 1; si < item.span[dir]; ++si) {
		 if (rs)
		   ts += RESIZE_HANDLE_MARGIN * 2;
		 rs = DC.config[di + rs][RESIZABLE] !== 0;
		 ts += DC.margins[SPACING];
		 ts += targetSize[di + si];
	       }
	     }

	     var off;

	     w.style.visibility = '';

	     var alignment = (item.align >> DC.alignBits) & 0xF;
	     var ps = item.ps[dir];

	     if (ts < ps)
	       alignment = 0;

	     if (!alignment) {
	       var m = margin(item.w, dir);

	       /*
		* Chrome:
		*  - some elements loose their margin as soon as you give
		*    them a size, we thus need to correct for that, or
		*    confirm their initial margin
		*/
	       var tsm = ts;
	       if (WT.isIElt9 ||
		   (!WT.hasTag(w, 'BUTTON') && !WT.hasTag(w, 'INPUT') &&
		    !WT.hasTag(w, 'SELECT') && !WT.hasTag(w, 'TEXTAREA')))
		 tsm = Math.max(0, tsm - m);

	       /*
		* IE: a button expands to parent container width ?? WTF ?
		*/
	       var setSize = false;
	       if (WT.isIE && WT.hasTag(w, 'BUTTON'))
		 setSize = true;

	       if (setSize || ts != ps || item.layout) {
		 if (setCss(w, DC.size, tsm + 'px'))
		   setItemDirty(item);
		 item.set[dir] = true;
	       } else {
		 if (!item.fs[dir]) {
		   if (setCss(w, DC.size, ''))
		     setItemDirty(item);
		   item.set[dir] = false;
		 } else if (dir == HORIZONTAL)
		   setCss(w, DC.size, item.fs[dir] + 'px');
	       }

	       off = left;
	       item.size[dir] = tsm;
	       item.psize[dir] = ts;
	     } else {
	       switch (alignment) {
	       case ALIGN_LEFT: off = left; break;
	       case ALIGN_CENTER: off = left + (ts - ps)/2; break;
	       case ALIGN_RIGHT: off = left + (ts - ps); break;
	       }

	       if (item.layout) {
		 if (setCss(w, DC.size, ps + 'px'))
		   setItemDirty(item);
		 item.set[dir] = true;
	       } else if (ts >= ps && item.set[dir]) {
		 if (setCss(w, DC.size, ps + 'px'))
		   setItemDirty(item);
		 item.set[dir] = false;
	       }

	       item.size[dir] = ps;
	       item.psize[dir] = ps;
	     }

	     if (!progressive)
	       setCss(w, DC.left, off + 'px');
	     else
	       if (thisResized) {
		 setCss(w, DC.left, (RESIZE_HANDLE_MARGIN * 2) + 'px');
		 var pc = WT.css(w, 'position');
		 if (pc !== 'absolute')
		   w.style.position = 'relative';
	       } else
		 setCss(w, DC.left, '0px');

	     if (dir == VERTICAL) {
	       if (w.wtResize)
		 w.wtResize(w,
			    Math.round(item.size[HORIZONTAL]),
			    Math.round(item.size[VERTICAL]));

	       item.dirty = false;
	     }
	   }
	 }

	 left += targetSize[di];
       }
     }
   }

   this.setConfig = function(conf) {
     /*
      * Flush preferred size measurements
      */
     var oldConfig = config;
     config = conf;

     DirConfig[0].config = config.cols;
     DirConfig[1].config = config.rows;

     var i, il, childLayouts = {};
     for (i = 0, il = oldConfig.items.length; i < il; ++i) {
       var item = oldConfig.items[i];

       if (item) {
	 if (item.set[HORIZONTAL])
	   setCss(item.w, DirConfig[HORIZONTAL].size, '');
	 if (item.set[VERTICAL])
	   setCss(item.w, DirConfig[VERTICAL].size, '');

	 if (item.layout) {
	   self.setChildSize(item.w, HORIZONTAL, item.ps[HORIZONTAL]);
	   self.setChildSize(item.w, VERTICAL, item.ps[VERTICAL]);
	 }
       }
     }

     layoutDirty = true;
   };

   this.getId = function() {
     return id;
   };

   this.setItemsDirty = function(items) {
     var i, il;

     var colCount = DirConfig[HORIZONTAL].config.length;
     for (i = 0, il = items.length; i < il; ++i) {
       var row = items[i][0], col = items[i][1];
       config.items[row * colCount + col].dirty = true;
     }

     itemDirty = true;
   };

   this.setDirty = function() {
     layoutDirty = true;
   };

   this.setChildSize = function(widget, dir, preferredSize) {
     var i, il;
     for (i = 0, il = config.items.length; i < il; ++i) {
       var item = config.items[i];
       if (item && item.id == widget.id) {
	 if (!item.ps)
	   item.ps = [];

	 item.ps[dir] = preferredSize;
	 item.layout = true;
	 break;
       }
     }

     itemDirty = true;
   };

   this.measure = function(dir) {
     var widget = WT.getElement(id);

     if (!widget)
       return;

     if (WT.isHidden(widget))
       return;

     if (!parentInitialized) {
       parentInitialized = true;
       topLevel = parentId == null;

       if (!topLevel) {
	 parent = jQuery.data(document.getElementById(parentId), 'layout');
	 parentItemWidget = widget;
	 parentMargin[HORIZONTAL] = boxMargin(parentItemWidget, HORIZONTAL);
	 parentMargin[VERTICAL] = boxMargin(parentItemWidget, VERTICAL);
       } else {
	 /*
	  * While we are a single child in a parent, we can go further
	  * up looking for an ancestor layout
	  */
	 var c = widget, p = c.parentNode;

	 parentMargin = [0, 0];
	 for (;;) {
	   parentMargin[HORIZONTAL] += boxMargin(p, HORIZONTAL);
	   parentMargin[VERTICAL] += boxMargin(p, VERTICAL);

	   if (p.childNodes.length > 1) {
	     var i, il;
	     for (i = 0, il = p.childNodes.length; i < il; ++i) {
	       if (p.childNodes[i] != c) {
		 parentMargin[VERTICAL] += p.childNodes[i].offsetHeight;
		 parentMinPW
		   = Math.max(p.childNodes[i].offsetWidth, parentMinPW);
	       }
	     }
	   }

	   var l = jQuery.data(p.parentNode, 'layout');
	   if (l) {
	     parent = l;
	     parentItemWidget = p;
	     break;
	   }

	   c = p;
	   p = c.parentNode;
	   if (p.childNodes.length != 1)
	     break;
	 }
       }
     }

     if (itemDirty || layoutDirty) {
       var container = topLevel ? widget.parentNode : null;
       measure(dir, widget, container);
     }

     if (dir == VERTICAL)
       itemDirty = layoutDirty = false;
   };

   this.apply = function(dir) {
     var widget = WT.getElement(id);

     if (!widget)
       return false;

     if (WT.isHidden(widget))
       return true;

     apply(dir, widget);

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
(2, JavaScriptPrototype, "StdLayout2.prototype.initResize",
 function(WT, id, config) {
   var self = this;

   self.resizeInitialized = true;
 });

WT_DECLARE_APP_MEMBER
(1, JavaScriptObject, "layouts2",
  new (function() {
    var topLevelLayouts = [], adjusting = false;
    var self = this;
    var measureVertical = false;

    this.find = function(id) {
      return jQuery.data(document.getElementById(id), 'layout');
    };

    this.setDirty = function(layoutEl) {
      var layout = jQuery.data(layoutEl, 'layout');

      if (layout)
	layout.setDirty();
    };

    this.add = function(layout) {
      function addIn(layouts, layout) {
	var i, il;

	for (i=0, il = layouts.length; i < il; ++i) {
	  var ll = layouts[i];

	  if (ll.contains(layout)) {
	    addIn(ll.descendants, layout);
	    return;
	  } else if (layout.contains(ll)) {
	    layout.descendants.push(ll);
	    layouts.splice(i, 1);
	    --i; --il;
	  }
	}

	layouts.push(layout);
      }

      addIn(topLevelLayouts, layout);
    };

    var adjustScheduled = false;

    this.scheduleAdjust = function() {
      if (adjustScheduled)
	return;

      adjustScheduled = true;

      setTimeout(function() { self.adjust(); }, 0);
    };

    this.adjust = function(id, items) {
      if (id) {
	var layout = this.find(id);
	if (layout)
	  layout.setItemsDirty(items);

	self.scheduleAdjust();

	return;
      }

      adjustScheduled = false;

      if (adjusting)
	return;

      adjusting = true;

      function measure(layouts, dir) {
	var i, il;

	for (i = 0, il = layouts.length; i < il; ++i) {
	  var ll = layouts[i];

	  measure(ll.descendants, dir);

	  if (dir == VERTICAL && measureVertical)
	    ll.setDirty();

	  ll.measure(dir);
	}
      }

      function apply(layouts, dir) {
	var i, il;

	for (i = 0, il = layouts.length; i < il; ++i) {
	  var ll = layouts[i];

	  if (!ll.apply(dir)) {
	    layouts.splice(i, 1);
	    --i; --il;
	  } else
	    apply(ll.descendants, dir);
	}
      }

      /** @const */ var HORIZONTAL = 0;
      /** @const */ var VERTICAL = 1;

      measure(topLevelLayouts, HORIZONTAL);
      apply(topLevelLayouts, HORIZONTAL);
      measure(topLevelLayouts, VERTICAL);
      apply(topLevelLayouts, VERTICAL);

      adjusting = false;
      measureVertical = false;
    };

    this.updateConfig = function(id, config) {
      var layout = this.find(id);
      if (layout)
	layout.setConfig(config);

      return;
    };

    window.onresize = function() {
      measureVertical = true;

      self.scheduleAdjust();
    };
  }) ()
);
