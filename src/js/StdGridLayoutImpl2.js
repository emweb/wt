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

   /** @const */ var SPACING = 0;
   /** @const */ var MARGIN_LEFT = 1;
   /** @const */ var MARGIN_RIGHT = 2;

   /** @const */ var PREFERRED_SIZE = 0;
   /** @const */ var MINIMUM_SIZE = 1;
   /** @const */ var TOTAL_PREFERRED_SIZE = 2;
   /** @const */ var TOTAL_MINIMUM_SIZE = 3;

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
     parentInitialized = false;

   var DirConfig = 
     [ {
         initialized: false,
	 config: config.cols,
	 margins: hMargins,
	 maxSize: maxWidth,
	 measures: [],
	 sizes: [],
	 fixedSize: [],
	 Left: 'Left',
	 left: 'left',
	 Right: 'Right',
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
	 alignBits: 8,
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

     /*
      * Firefox adds the -NA offset to the reported width ??
      */
     if (clientWidth > NA) {
       clientWidth -= NA;
       if (scrollWidth > NA)
	 scrollWidth -= NA;
     }

     if (scrollWidth == clientWidth) {
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

     return scrollWidth;
   }

   function calcMinimumSize(element, dir) {
     var DC = DirConfig[dir];

     if (element.style.display == 'none'
	 || element.style.visibility == 'hidden')
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

   function setItemDirty(item) {
     item.dirty = true;
     itemDirty = true;
     APP.layouts2.scheduleAdjust();
   };

   function measure(dir, widget, container)
   {
     var DC = DirConfig[dir],
         measures = DC.measures,
         dirCount = DC.config.length,
         otherCount = DirConfig[dir ^ 1].config.length,
         maxSize = DC.maxSize;

     if (!itemDirty && !layoutDirty)
       return;

     var prevMeasures = measures.slice();
     if (prevMeasures.length == 5) {
       prevMeasures[PREFERRED_SIZE] = prevMeasures[PREFERRED_SIZE].slice();
       prevMeasures[MINIMUM_SIZE] = prevMeasures[MINIMUM_SIZE].slice();
     }

     var preferredSize = [], minimumSize = [],
       totalPreferredSize = 0, totalMinSize = 0, di;

     var measurePreferredForStretching = DC.maxSize > 0 || !DC.initialized[dir];

     for (di = 0; di < dirCount; ++di) {
       var dPreferred = 0;
       var dMinimum = DC.config[di][MIN_SIZE];
       var allHidden = true;

       for (oi = 0; oi < otherCount; ++oi) {
	 var item = DC.getItem(di, oi);
	 if (item) {
	   if (!item.w) {
	     var $w = $("#" + item.id);
	     item.w = $w.get(0);
	     var citem = item;
	     $w.find("img").load(function() { setItemDirty(citem); });
	     item.w.style.left = item.w.style.top = NA_px;
	   }

	   if (!item.ps)
	     item.ps = [];

	   if (!item.ws)
	     item.ws = [];

	   if (!item.size)
	     item.size = [];

	   if (!item.set)
	     item.set = [false, false];

	   if (item.w) {
	     if (WT.isIE)
	       item.w.style.visibility = '';

	     if (debug)
	       console.log("measure " + dir + " "
			   + item.id + ': ' + item.ps[0] + ',' + item.ps[1]);
	     if (item.dirty || layoutDirty) {
	       var alignment = (item.align >> DC.alignBits) & 0xF;

	       if (alignment
		   || measurePreferredForStretching
		   || (DC.config[di][STRETCH] <= 0)) {
		 var wPreferred = calcPreferredSize(item.w, dir);

		 if (item.layout && (wPreferred < item.ps[dir]))
		   wPreferred = item.ps[dir];
	         else {
		   item.ps[dir] = wPreferred;
		 }

		 if (!item.span || item.span[dir] == 1)
		   if (wPreferred > dPreferred)
		     dPreferred = wPreferred;
	       }

	       var wMinimum = calcMinimumSize(item.w, dir);
	       if (wMinimum > dMinimum) 
		 dMinimum = wMinimum;
	       item.ws[dir] = wMinimum;

	       if (dir == VERTICAL)
		 item.dirty = false;
	     } else {
	       if (!item.span || item.span[dir] == 1)
		 if (item.ps[dir] > dPreferred)
		   dPreferred = item.ps[dir];
	       if (item.ws[dir] > dMinimum)
		 dMinimum = item.ps[dir];
	     }

	     if (debug)
	       console.log(" ->" + item.id + ': ' + item.ps[0] + "," + item.ps[1]);

	     if (item.w.style.display !== 'none')
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

	 rh = DC.config[di][RESIZABLE];
       }
     }

     if (!first)
       totalMargin += DC.margins[MARGIN_RIGHT];

     totalPreferredSize += totalMargin;
     totalMinSize += totalMargin;

     DC.measures = [
	     preferredSize,
	     minimumSize,
	     totalPreferredSize,
	     totalMinSize
	     ];

     /*
      * If we are directly in a parent layout, then we want to
      * mark the corresponding cell as dirty if the TOTAL_PREFERRED_SIZE
      * has changed.
      */
     if (parent) {
       if (prevMeasures[TOTAL_PREFERRED_SIZE]
	   != DC.measures[TOTAL_PREFERRED_SIZE]) {
	 var margin = boxMargin(parentItemWidget, dir);
	 parent.setChildSize(parentItemWidget, dir,
			     DC.measures[TOTAL_PREFERRED_SIZE] + margin);
       }
     }

     if (container
	 && prevMeasures[TOTAL_MINIMUM_SIZE]
	 != DC.measures[TOTAL_MINIMUM_SIZE]) {
       /*
	* If our minimum layout requirements have changed, then we want
	* to communicate this up using the minimum widths
	*  -- FIXME IE6
	*/
       var w = DC.measures[TOTAL_MINIMUM_SIZE] + 'px';
       if (container.style['min' + DC.Size] != w) {
	 container.style['min' + DC.Size] = w;
	 if (self.ancestor)
	   self.ancestor.setContentsDirty(container);
       }
     }
   }

   function finishResize(dir, di, delta) {
     /*
      * fix size of di at current size + delta
      */
     var DC = DirConfig[dir];
     DC.fixedSize[di] = DC.sizes[di] + delta;

     // FIXME: RTL

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
	     DC.initialized = true;
	     if (pc !== 'absolute') {
	       cSize = dir ? container.clientHeight : container.clientWidth;

	       cClientSize = true;

	       if (dir == 0 && cSize == 0 && WT.isIE6) {
		 cSize = container.offsetWidth;
		 cClientSize = false;
	       }

	       /*
		* heuristic to switch to layout-sizes-container mode
		*/
	       if ((WT.isIE6 && cSize == 0)
		   || (cSize == measures[TOTAL_MINIMUM_SIZE]
		       + padding(container, dir)))
		 DC.maxSize = 999999;
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

     if (DC.maxSize) {
       // (2) adjust container width/height
       if (measures[TOTAL_PREFERRED_SIZE] < DC.maxSize) {
	 container.style[DC.size] = measures[TOTAL_PREFERRED_SIZE] + 'px';
	 cSize = measures[TOTAL_PREFERRED_SIZE];
	 cPaddedSize = true;
	 noStretch = true;
       } else {
	 cSize = DC.maxSize;
	 cClientSize = false;
       }
     }

     if (!cPaddedSize) {
       if (cClientSize)
	 cSize -= padding(container, dir);
       else
	 cSize -= sizePadding(container, dir);
     }

     if (container && cSize <= 0)
       return;

     // (2a) if we can't satisfy minimum sizes, then overflow container
     if (cSize < measures[TOTAL_MINIMUM_SIZE])
       cSize = measures[TOTAL_MINIMUM_SIZE];

     $(widget).children("." + OC.handleClass)
       .css(DC.size,
	    (cSize - DC.margins[MARGIN_RIGHT] - DC.margins[MARGIN_LEFT])
	    + 'px');

     // (3) compute column/row widths
     var targetSize = measures[MINIMUM_SIZE].slice(),
       dirCount = DC.config.length,
       otherCount = OC.config.length;
     
     if (debug)
       console.log("apply " + id + ': '
		   + dir + " ps " + measures[PREFERRED_SIZE] + " cSize " + cSize);

     /*
      * Heuristic for nested layout with AlignLeft or AlignTop ?
      */
     if (cSize == measures[TOTAL_PREFERRED_SIZE])
       noStretch = true;

     if (cSize > measures[TOTAL_MINIMUM_SIZE]) {
       // non-stretchable colums/rows get up to their preferred size
       // excess space is distributed to stretchable column/rows
       // fixed size columns/rows get their fixed width

       var totalNonStretch = 0,
	 totalStretch = 0,
	 totalFixed = 0,
	 notFixedCount = 0;
       for (var di = 0; di < dirCount; ++di) {
	 if (measures[MINIMUM_SIZE][di] > -1) {
	   if (typeof DC.fixedSize[di] !== "undefined") {
	     totalFixed += DC.fixedSize[di];
	     targetSize[di] = DC.fixedSize[di];
	   } else {
	     ++notFixedCount;
	     if (DC.config[di][STRETCH] <= 0)
	       totalNonStretch += measures[PREFERRED_SIZE][di]
		 - measures[MINIMUM_SIZE][di];
	     else
	       totalStretch += DC.config[di][STRETCH];
	   }
	 }
       }

       var toDistribute = cSize - measures[TOTAL_MINIMUM_SIZE] - totalFixed;

       // if no column has stretch (and we aren't simply using preferred
       // sizes throughout), make them all (but the fixed size ones) stretch
       if (DC.fitSize && !noStretch && (totalStretch == 0))
	 totalNonStretch = 0;

       if (totalNonStretch) {
	 var nsDistribute;
	 if (toDistribute > totalNonStretch)
	   nsDistribute = totalNonStretch;
	 else
	   nsDistribute = toDistribute;

	 var nsFactor = nsDistribute / totalNonStretch;

	 for (var di = 0; di < dirCount; ++di) {
	   if (measures[MINIMUM_SIZE][di] > -1) {
	     if (typeof DC.fixedSize[di] === "undefined"
		 && DC.config[di][STRETCH] <= 0) {
	       var s = measures[PREFERRED_SIZE][di]
		 - measures[MINIMUM_SIZE][di];
	       targetSize[di] += nsFactor * s;
	     }
	   }
	 }

	 toDistribute -= nsDistribute;
       }

       if (DC.fitSize && toDistribute > 0) {
	 var ts = totalStretch;
	 if (totalStretch == 0)
	   ts = notFixedCount;

	 var factor = toDistribute / ts;

	 for (var di = 0; di < dirCount; ++di) {
	   if (measures[MINIMUM_SIZE][di] > -1) {
	     if (typeof DC.fixedSize[di] === "undefined") {
	       var stretch;
	       if (totalStretch == 0)
		 stretch = 1;
	       else
		 stretch = DC.config[di][STRETCH];

	       if (stretch > 0)
		 targetSize[di] += stretch * factor;
	     }
	   }
	 }
       }
     }

     DC.sizes = targetSize;

     if (debug)
       console.log(" -> targetSize: " + targetSize);

     // (4) set widths/heights of cells
     var left = 0, first = true, resizeHandle = false;

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
	     }
	   }

	   left += RESIZE_HANDLE_MARGIN;
	   handle.style[DC.left] = left + 'px';
	   left += RESIZE_HANDLE_MARGIN;
	 }

	 resizeHandle = DC.config[di][RESIZABLE];

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
		 rs = DC.config[di + rs][RESIZABLE];
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
	       var m = margin(w, dir);

	       /*
		* Chrome:
		*  - some elements loose their margin as soon as you give
		*    them a size, we thus need to correct for that, or
		*    confirm their initial margin
		*/
	       var tsm = ts;
	       if (WT.isIElt9 ||
		   (!WT.hasTag(w, 'BUTTON') && !WT.hasTag(w, 'INPUT') &&
		    !WT.hasTag(w, 'SELECT')))
		 tsm = Math.max(0, tsm - m);

	       /*
		* IE: a button expands to parent container width ?? WTF ?
		*/
	       var setSize = false;
	       if (WT.isIE && WT.hasTag(w, 'BUTTON'))
		 setSize = true;

	       if (setSize || ts != ps || w.style[DC.size] != '' || item.layout) {
		 w.style[DC.size] = tsm + 'px';
		 item.set[dir] = true;
	       } else {
		 w.style[DC.size] = '';
	       }

	       off = left;
	       item.size[dir] = tsm;
	     } else {
	       var off;
	       switch (alignment) {
	       case ALIGN_LEFT: off = left; break;
	       case ALIGN_CENTER: off = left + (ts - ps)/2; break;
	       case ALIGN_RIGHT: off = left + (ts - ps); break;
	       }

	       if (item.layout)
		 w.style[DC.size] = ps + 'px';

	       item.size[dir] = ps;
	     }

	     if (!progressive)
	       w.style[DC.left] = off + 'px';
	     else
	       if (thisResized) {
		 w.style[DC.left] = (RESIZE_HANDLE_MARGIN * 2) + 'px';
		 var pc = WT.css(w, 'position');
		 if (pc !== 'absolute')
		   w.style.position = 'relative';
	       } else
		 w.style[DC.left] = '0px';		 

	     if (dir == VERTICAL && w.wtResize) {
	       w.wtResize(w, item.size[HORIZONTAL], item.size[VERTICAL]);
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
	   item.w.style[DirConfig[HORIZONTAL].size] = '';
	 if (item.set[VERTICAL])
	   item.w.style[DirConfig[VERTICAL].size] = '';

	 if (item.layout) {
	   self.setChildSize(item.w, HORIZONTAL, item.ps[HORIZONTAL]);
	   self.setChildSize(item.w, VERTICAL, item.ps[VERTICAL]);
	 }
       }
     }

     layoutDirty = true;
   }

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
   }

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
   }

   this.measure = function(dir) {
     var widget = WT.getElement(id);
     if (!widget)
       return false;

     if (!parentInitialized) {
       parentInitialized = true;
       topLevel = parentId == null;

       if (!topLevel) {
	 parent = jQuery.data(document.getElementById(parentId), 'layout');
	 parentItemWidget = widget;
       } else {
	 parent = jQuery.data(widget.parentNode.parentNode, 'layout');
	 if (parent)
	   parentItemWidget = widget.parentNode;
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

    this.find = function(id) {
      return jQuery.data(document.getElementById(id), 'layout');
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

      setTimeout(function() { self.adjust(); }, 1);
    }

    this.adjust = function(id, items) {
      if (id) {
	var layout = this.find(id);
	if (layout)
	  layout.setItemsDirty(items);

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
    };

    this.updateConfig = function(id, config) {
      var layout = this.find(id);
      if (layout)
	layout.setConfig(config);

      return;
    }
  }) ()
);
