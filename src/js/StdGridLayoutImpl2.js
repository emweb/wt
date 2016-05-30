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
   var itemDirty = true;   /* one or more items (with .dirty=true) need to
			      be remeasured */
   var layoutDirty = true; /* the parent size may have changed */

   var topLevel, parent, parentItemWidget, propagateSizeToParent,
     parentInitialized = false, parentMargin, parentWithWtPS;

   var rtl = $(document.body).hasClass('Wt-rtl');

   var DirConfig =
     [ {
         initialized: false,
	 config: config.cols,
	 margins: hMargins,
	 maxSize: maxWidth,
	 measures: [],
	 sizes: [],
	 stretched: [],
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
	 setItem: function (ci, ri, item) {
	   config.items[ri * DirConfig[HORIZONTAL].config.length + ci] = item;
	 },
	 handleClass: 'Wt-vrh2',
	 resizeDir: 'h',
	 resizerClass: 'Wt-hsh2',
	 fitSize: fitWidth,
         resizeHandles: []
       }, {
         initialized: false,
	 config: config.rows,
	 margins: vMargins,
	 maxSize: maxHeight,
	 measures: [],
	 sizes: [],
	 stretched: [],
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
	 setItem: function (ri, ci, item) {
	   config.items[ri * DirConfig[HORIZONTAL].config.length + ci] = item;
	 },
	 handleClass: 'Wt-hrh2',
	 resizeDir: 'v',
	 resizerClass: 'Wt-vsh2',
	 fitSize: fitHeight,
         resizeHandles: []
       }];

   jQuery.data(document.getElementById(id), 'layout', this);

   function getItem(id) {
     var i, il;
     for (i = 0, il = config.items.length; i < il; ++i) {
       var item = config.items[i];
       if (item && item.id == id)
	  return item;
     }

     return null;
   }

   function calcPreferredSize(element, dir, asSet, widget) {
     var DC = DirConfig[dir];

     var scrollSize = dir ? element.scrollHeight : element.scrollWidth;

     var l, p;

     /* Allow accurate width measurement for widget with offset */
     if (dir == HORIZONTAL) {
       /*
	* Somehow, Firefox more correctly does not ignore parent width...
	*/
       var offLeft = WT.pxself(element, DC.left);
       if (scrollSize + offLeft > widget.clientWidth ||
	   (scrollSize + offLeft == widget.clientWidth &&
	    WT.isGecko &&
	    widget.parentNode.parentNode.style.visibility === 'hidden')) {
	 l = element.style[DC.left];
	 setCss(element, DC.left, NA_px);
       
	 scrollSize = dir ? element.scrollHeight : element.scrollWidth;
       }
     }

     var clientSize = dir ? element.clientHeight : element.clientWidth;
     /*
      * Firefox sets scrollSize = clientSize when there are no scrollbars
      * configured (overflow == 'visible' or 'none') on the widget.
      * But setting merely overflow does not help. Removing
      * the size constraint temporarily does work.
      */
     function isNone(overflow) {
       return overflow == 'visible' || overflow == 'none';
     }

     if (WT.isGecko &&
	 !element.style[DC.size] &&
	 dir == HORIZONTAL &&
	 isNone(WT.css(element, 'overflow'))) {
       p = element.style[DC.size];
       setCss(element, DC.size, '');
     }
     var offsetSize = dir ? element.offsetHeight : element.offsetWidth;

     if (l)
       setCss(element, DC.left, l);

     if (p)
       setCss(element, DC.size, p);

     /*
      * Firefox sometimes adds the -NA offset to the reported size
      */
     if (clientSize >= NA)
       clientSize -= NA;
     if (scrollSize >= NA)
       scrollSize -= NA;
     if (offsetSize >= NA)
       offsetSize -= NA;

     if (scrollSize === 0) {
       scrollSize = WT.pxself(element, DC.size);

       if (scrollSize !== 0 && !WT.isOpera && !WT.isGecko)
	 scrollSize -=
	   WT.px(element, 'border' + DC.Left + 'Width') +
	   WT.px(element, 'border' + DC.Right + 'Width');
     }

     if (scrollSize === clientSize) {
       // might be too big, investigate children
       // TODO
     }

     if (WT.isIE &&
	 (WT.hasTag(element, 'BUTTON') || WT.hasTag(element, 'TEXTAREA')
	  || WT.hasTag(element, 'INPUT') || WT.hasTag(element, 'SELECT'))) {
       scrollSize = clientSize;
     }

     if (scrollSize > offsetSize) {
       /*
	* could be because of a nested absolutely positioned element
	* with z-index > 0 ?, or could be because of genuine contents that is
	* not fitting the parent.
	*
	* the latter case is only possible if a size is currently set on the
	* element
	*/
       if (WT.pxself(element, DC.size) == 0)
	 scrollSize = clientSize;
       else {
	 var visiblePopup = false;
	 $(element).find(".Wt-popup").each
	   (function(index) {
	     if (this.style.display !== 'none')
	       visiblePopup = true;
	   });
	 if (visiblePopup)
	   scrollSize = clientSize;
       }
     }

     var border = WT.px(element, 'border' + DC.Left + 'Width') +
	 WT.px(element, 'border' + DC.Right + 'Width');

     var scrollbar = offsetSize - (clientSize + border) != 0;

     if (asSet)
       return [scrollSize, scrollBar];

     if ((WT.isGecko || WT.isWebKit) &&
	 dir == HORIZONTAL && 
	 element.getBoundingClientRect().width != 
	 Math.ceil(element.getBoundingClientRect().width))
       scrollSize += 1;

     if (!WT.boxSizing(element) && !WT.isOpera)
       scrollSize += border;

     scrollSize +=
       WT.px(element, 'margin' + DC.Left) +
       WT.px(element, 'margin' + DC.Right);

     if (!WT.boxSizing(element) && !WT.isIE)
       scrollSize +=
	 WT.px(element, 'padding' + DC.Left) +
	 WT.px(element, 'padding' + DC.Right);

     /* Must be because of a scrollbar */
     scrollSize += (offsetSize - (clientSize + border));

     if (scrollSize < offsetSize) // happens on IE8?
       scrollSize = offsetSize;

     var maxSize = WT.px(element, 'max' + DC.Size);
     if (maxSize > 0)
       scrollSize = Math.min(maxSize, scrollSize);

     return [Math.round(scrollSize), scrollbar];
   }

   function calcMinimumSize(element, dir) {
     var DC = DirConfig[dir];

     if (element.style.display === 'none')
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

     return Math.round(WT.px(el, 'border' + DC.Left + 'Width') +
		       WT.px(el, 'border' + DC.Right + 'Width') +
		       WT.px(el, 'margin' + DC.Left) +
		       WT.px(el, 'margin' + DC.Right) +
		       WT.px(el, 'padding' + DC.Left) +
		       WT.px(el, 'padding' + DC.Right));
   }

   /*
    * dirt: 0 = not dirty
    *       1 = remeasure size
    *       2 = remeasure all
    */
   function setItemDirty(item, dirt, scheduleAdjust) {
     item.dirty = Math.max(item.dirty, dirt);
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

   /*
    * Only checks the single widget -- already assumes the ancestors are visible
    */
   function isHidden(w) {
     // XXX second condition is a hack for WTextEdit
     return (w.style.display === 'none' && !w.ed) || $(w).hasClass('Wt-hidden');
   }

   function measure(dir, widget, container)
   {
     var DC = DirConfig[dir],
         OC = DirConfig[dir ^ 1],
         measures = DC.measures,
         dirCount = DC.config.length,
         otherCount = OC.config.length;

     var prevMeasures = measures.slice();
     if (prevMeasures.length == 5) {
       prevMeasures[PREFERRED_SIZE] = prevMeasures[PREFERRED_SIZE].slice();
       prevMeasures[MINIMUM_SIZE] = prevMeasures[MINIMUM_SIZE].slice();
     }

     /*
      * If only layoutDirty then we do not need to remeasure children,
      * but simply propagate up again the total preferred size and reapply the
      * layout as the container may have changed in size.
      */
     if (itemDirty) {

     if (container && typeof DC.minSize == 'undefined') {
       DC.minSize = WT.px(container, 'min' + DC.Size);
       if (DC.minSize > 0)
	 DC.minSize -= sizePadding(container, dir);
     }

     var preferredSize = [], minimumSize = [], di, oi;

     var measurePreferredForStretching = true, spanned = false;

     for (di = 0; di < dirCount; ++di) {
       var dPreferred = 0;
       var dMinimum = DC.config[di][MIN_SIZE];
       var allHidden = true;

       for (oi = 0; oi < otherCount; ++oi) {
	 var item = DC.getItem(di, oi);

	 if (item) {
	   if (!item.w || (dir == HORIZONTAL && item.dirty > 1)) {
	     var $w = $("#" + item.id);
	     var w2 = $w.get(0);

	     if (!w2) { // is missing, could be because it is overspanned!
	       DC.setItem(di, oi, null);
	       continue;
	     }

	     if (w2 != item.w) {
	       item.w = w2;

	       $w.find("img").add($w.filter("img"))
		 .bind('load',
		       { item: item},
		       function(event) {
			 setItemDirty(event.data.item, 1, true);
		       });
	     }
	   }

	   if (!progressive && item.w.style.position != 'absolute') {
	     item.w.style.position = 'absolute';
	     item.w.style.visibility = 'hidden';
	   }

	   if (!item.ps)
	     item.ps = []; // preferred size

	   if (!item.sc)
	     item.sc = []; // scrollbar (if present needs forceful size set)
	     
	   if (!item.ms)
	     item.ms = []; // minimum size

	   if (!item.size)
	     item.size = []; // set size

	   if (!item.psize)
	     item.psize = []; // set size (incl. margins, like preferred size)

	   if (!item.fs)
	     item.fs = []; // fixed size (size defined by inline size or CSS)

	   if (!item.margin)
	     item.margin = [];

	   var first = !item.set;

	   if (!item.set)
	     item.set = [false, false];

	   if (isHidden(item.w)) {
	     item.ps[dir] = item.ms[dir] = 0;
	     continue;
	   }

	   if (item.w) {
	     if (WT.isIE)
	       item.w.style.visibility = '';

	     if (debug)
	       console.log("measure " + dir + " "
	 		   + item.id +
			   (item.w.className ? ' (' + item.w.className + '): '
			   : ': ') +
			    item.ps[0] + ',' + item.ps[1]
			   + ',' + item.dirty + ', set: ['
			   + item.set[0] + ',' + item.set[1] + '], sc: ['
			   + item.sc[0] + ',' + item.sc[1] + ']');

	     if (item.dirty) {
	       var wMinimum;
	       if (item.dirty > 1) {
		 wMinimum = calcMinimumSize(item.w, dir);
		 item.ms[dir] = wMinimum;
	       } else
		 wMinimum = item.ms[dir];

	       /*
		* if we do not have an size set, we can and should take into
		* account the size set for a widget by CSS. But we can't really
		* read this -- computedStyle for width or height measures
		* instead of interpreting the stylesheet ... !
		*
		* we'll consider computedStyle only for vertical size, and only
		* the first time
		*/
	       if (item.dirty > 1)
		 item.margin[dir] = margin(item.w, dir);

               if (!item.set[dir]) {
		 if (dir == HORIZONTAL || !first) {
		   var fw = WT.pxself(item.w, DC.size);
		   if (fw)
		     item.fs[dir] = fw + item.margin[dir];
		   else
		     item.fs[dir] = 0;
		 } else {
		   var fw = Math.round(WT.px(item.w, DC.size));
		   if (fw > Math.max(sizePadding(item.w, dir), wMinimum))
		     item.fs[dir] = fw + item.margin[dir];
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
		   if (item.wasLayout) {
		     item.wasLayout = false;
		     item.set = [false, false];
		     item.ps = [];
		     if (item.w.wtResize)
		       item.w.wtResize(item.w, -1, -1, true);
		     setCss(item.w, DirConfig[1].size, '');
		   }

		   var calc = calcPreferredSize(item.w, dir, false, widget);

		   var calculated = calc[0];

		   /*
		    * If we've set the size then we should not take the
		    * set size as the preferred size, instead we revert
		    * to a previous preferred size.
		    */
		   var sizeSet = item.set[dir];
		   if (sizeSet) {
		     /*
		      * Make an exception for very small sizes set ...
		      * Typically to discount for margin/paddings on
		      * form elements which are not properly measured
		      * if the item is being reduced to very small
		      * width/height
		      */
		     if (item.psize[dir] > 8)
		       sizeSet = (calculated >= item.psize[dir] - 4) &&
			 (calculated <= item.psize[dir] + 4);
		   }

		   /*
		    * If this is an item that is stretching and has
		    * been stretched (item.set[dir]), then we should
		    * not remeasure the preferred size since it might
		    * confuse the user with constant resizing.
		    *
		    * -- FIXME: what if sum stretch=0, we are actually
		    * also stretching ?
		    */
		   var stretching = (typeof item.ps[dir] !== 'undefined')
		     && (DC.config[di][STRETCH] > 0)
		     && item.set[dir];


		   if (sizeSet || stretching) 
		     wPreferred = Math.max(wPreferred, item.ps[dir]);
		   else 
		     wPreferred = Math.max(wPreferred, calculated);

		   item.ps[dir] = wPreferred;
		   item.sc[dir] = calc[1];
		 } else {
		   if (wPreferred == 0)
		     wPreferred = item.ps[dir];
		   item.ps[dir] = wPreferred;
		 }

	       } else if (item.layout) {
		 if (wPreferred == 0)
		   wPreferred = item.ps[dir];
	       }

	       if (!item.span || item.span[dir] == 1) {
		 if (wPreferred > dPreferred)
		   dPreferred = wPreferred;
		 if (wMinimum > dMinimum)
		   dMinimum = wMinimum;
	       } else
		 spanned = true;
	     } else {
	       if (!item.span || item.span[dir] == 1) {
		 if (item.ps[dir] > dPreferred)
		   dPreferred = item.ps[dir];
		 if (item.ms[dir] > dMinimum)
		   dMinimum = item.ms[dir];
	       } else
		 spanned = true;
	     }

	     if (debug)
	       console.log(" ->" + item.id + ': ' + item.ps[0] + ","
			   + item.ps[1]);

	     var hidden = isHidden(item.w);

	     if (!hidden && (!item.span || item.span[dir] == 1)) {
	       allHidden = false;
	     }
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

     if (spanned) {
       if (debug)
	 console.log("(before spanned) "
		     + id + ': ' + dir + " ps " + preferredSize + " ms " + minimumSize);

       function handleOverspanned(getItemSize, sizes) {
	 for (di = dirCount - 1; di >= 0; --di) {
	   for (oi = 0; oi < otherCount; ++oi) {
	     var item = DC.getItem(di, oi);

	     if (item && item.span && item.span[dir] > 1) {
	       var ps = getItemSize(item), count = 0, stretch = 0, si;

	       for (si = 0; si < item.span[dir]; ++si) {
		 var cps = sizes[di + si];

		 if (cps != -1) {
		   ps -= cps;
		   ++count;
		   if (DC.config[di + si][STRETCH] > 0)
		     stretch += DC.config[di + si][STRETCH];

		   if (si != 0)
		     ps -= DC.margins[SPACING];
		 }
	       }

	       if (ps >= 0) {
		 if (count > 0) {
		   if (stretch > 0)
		     count = stretch;

		   for (si = 0; si < item.span[dir]; ++si) {
		     var cps = sizes[di + si];
		     if (cps != -1) {
		       var portion;
		       if (stretch > 0)
			 portion = DC.config[di + si][STRETCH];
		       else
			 portion = 1;

		       if (portion > 0) {
			 var fract = Math.round(ps * (portion / count));
			 ps -= fract; count -= portion;
			 sizes[di + si] += fract;
		       }
		     }
		   }
		 } else
		   sizes[di] = ps;
	       }
	     }
	   }
	 }
       }
	
       handleOverspanned(function(item) { return item.ps[dir]; },
			 preferredSize);

       handleOverspanned(function(item) { return item.ms[dir]; },
			 minimumSize);
     }


     var totalPreferredSize = 0, totalMinSize = 0;

     for (di = 0; di < dirCount; ++di) {
       if (minimumSize[di] > preferredSize[di])
	 preferredSize[di] = minimumSize[di];

       if (minimumSize[di] > -1) {
	 totalPreferredSize += preferredSize[di];
	 totalMinSize += minimumSize[di];
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
       console.log("measured " + id + ': ' + dir + " ps " + preferredSize + " ms " + minimumSize);

     DC.measures = [
	     preferredSize,
	     minimumSize,
	     totalPreferredSize,
	     totalMinSize,
	     totalMargin
	     ];
     } // itemDirty

     if (layoutDirty ||
	 (prevMeasures[TOTAL_PREFERRED_SIZE] !=
	  DC.measures[TOTAL_PREFERRED_SIZE]))
	 self.updateSizeInParent(dir);

     /* If our minimum layout requirements have changed, then we want
      * to communicate this up using the minimum widths
      *  -- FIXME IE6
      */
     if (container
	 && DC.minSize == 0
	 && prevMeasures[TOTAL_MINIMUM_SIZE] != DC.measures[TOTAL_MINIMUM_SIZE]
	 && container.parentNode.className != 'Wt-domRoot') {
       var w = DC.measures[TOTAL_MINIMUM_SIZE] + 'px';
       setCss(container, 'min' + DC.Size, w);
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

   this.updateSizeInParent = function(dir) {
     /*
      * If we are directly in a parent layout, then we want to
      * mark the corresponding cell as dirty if the TOTAL_PREFERRED_SIZE
      * has changed (or force).
      */
     if (parent && parentItemWidget.id) {
       var piw = WT.$(parentItemWidget.id);
       if (piw) {
	 if (parentItemWidget != piw) {
	   parent = jQuery.data(piw.parentNode, 'layout');
	   if (!parent) {
	     /* The parent item widget is no longer in the DOM. Need a test
	      * case for that. */
	     initializeParent();
	   } else
	     parentItemWidget = piw;
	 }
       } else {
	 /* The parent item widget is no longer in the DOM. Need a test case
	  * for that. */
	 initializeParent();
       }
     }

     if (parent) {
       if (propagateSizeToParent) {
	 var DC = DirConfig[dir],
           totalPs = DC.measures[TOTAL_PREFERRED_SIZE];

	 if (DC.maxSize > 0)
	   totalPs = Math.min(DC.maxSize, totalPs);

	 if (parentWithWtPS) {
	   var widget = WT.getElement(id);

	   if (!widget)
	     return;

	   /*
	    * Go back up and apply wtGetPS() to totalPs
	    */
	   var c = widget, p = c.parentNode;

	   for (;;) {
	     if (p.wtGetPS)
	       totalPs = p.wtGetPS(p, c, dir, totalPs);

	     totalPs += boxMargin(p, dir);

	     if (p == parentItemWidget)
	       break;

	     if (dir == VERTICAL &&
		 p == widget.parentNode && 
		 !p.lh && 
		 p.offsetHeight > totalPs)
	       totalPs = p.offsetHeight;

	     c = p;
	     p = c.parentNode;
	   }
	 } else
	   totalPs += parentMargin[dir];

	 parent.setChildSize(parentItemWidget, dir, totalPs);
       }
     }
   };

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

   // does this column/row have an adjacent handle?
   function hasResizeHandle(DC, di) {
     var dirCount = DC.config.length;

     var isResizable = DC.config[di][RESIZABLE] !== 0;
     var i;

     if (isResizable) {
       /* Check that we are not the last column/row */
       for (i = di + 1; i < dirCount; ++i) {
	 if (DC.measures[MINIMUM_SIZE][i] > -1)
	   return true;
       }
     }

     /* See if preceding column/row has a resize handle */
     for (i = di - 1; i >= 0; --i) {
       if (DC.measures[MINIMUM_SIZE][i] > -1) {
	 return DC.config[i][RESIZABLE] !== 0;
       }
     }

     return false;
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

	 var sizedByAnimation = dir == 1 &&
	       typeof container.nativeHeight !== 'undefined';

	 if (pc === 'absolute') {
	   if (sizedByAnimation) {
	     cSize = WT.parsePx(container.nativeHeight);
	   } else
	     cSize = WT.pxself(container, DC.size);
         }

	 if (cSize === 0) {
	   if (!DC.initialized) {
	     if (dir === HORIZONTAL && (pc === 'absolute' || pc === 'fixed')) {
	       /*
		* On Chrome, somehow clientWidth is not reliable until
		* we do this little hocus pocus
		*/
	       container.style.display = 'none';
	       cSize = container.clientWidth;
	       container.style.display = '';
	     }

	     if (!sizedByAnimation)
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

	     if ((WT.hasTag(container, "TD") || WT.hasTag(container, "TH") ||
		  $(container.parentNode).hasClass('Wt-domRoot'))
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

	     if (debug)
	       console.log('cSize : ' + cSize
			   + (cClientSize ? '(clientSize)' : '') + ', ieCSize : '
			   + ieCSize
			   + ', minSize : ' + minSize + ', padding: '
			   + padding(container, dir) + ', sizePadding: '
			   + sizePadding(container, dir));

	     function epsNotLarger(a, b) {
	       return a - b <= 1;
	     }

	     if ((WT.isIElt9 && epsNotLarger(cSize, ieCSize))
		 || (epsNotLarger(cSize, minSize + padding(container, dir)))) {
	       if (debug)
		 console.log('switching to managed container size '
			     + dir + ' ' + id);
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
     } else {
       if (DC.sizeSet) {
	 cSize = WT.pxself(container, DC.size);
	 cPaddedSize = true;
       }
     }

     var otherPadding = 0;

     if (container && container.wtGetPS && dir == VERTICAL)
       otherPadding = container.wtGetPS(container, widget, dir, 0);

     var totalPreferredSize = measures[TOTAL_PREFERRED_SIZE];
     if (totalPreferredSize < DC.minSize)
       totalPreferredSize = DC.minSize;

     if (DC.maxSize && !DC.sizeSet) {
       // (2) adjust container width/height
       var sz = Math.min(totalPreferredSize, DC.maxSize) + otherPadding;
       if (setCss(container, DC.size,
		  (sz + sizePadding(container, dir)) + 'px')) {
	 if (parent)
	   parent.setElDirty(parentItemWidget);
       }

       cSize = sz;
       cPaddedSize = true;
       noStretch = true;
     }

     DC.cSize = cSize;

     if (dir == VERTICAL && container && container.wtResize) {
       var w = OC.cSize,
	   h = DC.cSize;
       container.wtResize(container, Math.round(w), Math.round(h), true);
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

     // (3) compute column/row widths
     var targetSize = [], dirCount = DC.config.length,
       otherCount = OC.config.length;

     for (di = 0; di < dirCount; ++di)
       DC.stretched[di] = false;

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

	   /*
	    * if resizable was disabled, remove fixedSize and go back to 
	    * using the preferred size
	    */
	   if (!hasResizeHandle(DC, di))
	     DC.fixedSize[di] = undefined;

	   /*
	    * If we have a fixedSize (set by resizing) then we should
	    * take it into account only if the resizer is still visible.
	    */
	   if (typeof DC.fixedSize[di] !== "undefined"
	       && (di + 1 == dirCount || measures[MINIMUM_SIZE][di + 1] > -1))
	     fs = DC.fixedSize[di];
	   else if (hasResizeHandle(DC, di) &&
		    DC.config[di][RESIZABLE] !== 0 &&
		    (DC.config[di][RESIZABLE][RS_INITIAL_SIZE] >= 0)) {
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
	 } else {
	   stretch[di] = HIDDEN;
	   targetSize[di] = -1;
	 }
       }

       if (DC.fixedSize.length > dirCount)
	 DC.fixedSize.length = dirCount;

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

	     var r = 0;
	     for (di = 0; di < dirCount; ++di) {
	       if (stretch[di] > 0) {
		 var oldr = r;
		 r += stretch[di] * factor;
		 targetSize[di] += Math.round(r) - Math.round(oldr);
		 DC.stretched[di] = true;
	       }
	     }
	   }
	 } else {
	   // shrink stretchables up to their minimum size

	   var category = STRETCHABLES;

	   if (toDistribute < totalMinimum[category])
	     toDistribute = totalMinimum[category];

	   var factor;

	   if (totalPreferred[category] - totalMinimum[category] > 0) {
	     factor = (toDistribute - totalMinimum[category])
	       / (totalPreferred[category] - totalMinimum[category]);
	   } else
	     factor = 0;

	   var r = 0;
	   for (di = 0; di < dirCount; ++di) {
	     if (stretch[di] > 0) {
	       var s = measures[PREFERRED_SIZE][di] 
		 - measures[MINIMUM_SIZE][di];
	       var oldr = r;
	       r += s * factor;
	       targetSize[di] = measures[MINIMUM_SIZE][di] 
		 + Math.round(r) - Math.round(oldr);
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

	 if (totalPreferred[category] - totalMinimum[category] > 0) {
	   factor = (toDistribute - totalMinimum[category])
	     / (totalPreferred[category] - totalMinimum[category]);
	 } else
	   factor = 0;

	 var r = 0;
	 for (di = 0; di < dirCount; ++di) {
	   if (stretch[di] == 0) {
	     var s = measures[PREFERRED_SIZE][di]
	       - measures[MINIMUM_SIZE][di];
	     var oldr = r;
	     r += s * factor;
	     targetSize[di] = measures[MINIMUM_SIZE][di]
	       + Math.round(r) - Math.round(oldr);
	   }
	 }
       }
     } else
       targetSize = measures[MINIMUM_SIZE];

     DC.sizes = targetSize;

     if (debug)
       console.log(" -> targetSize: " + targetSize);

     // (4) set widths/heights of cells
     var left = DC.margins[MARGIN_LEFT], 
       first = true, resizeHandle = false, oi, di;

     for (di = 0; di < dirCount; ++di) {
       if (targetSize[di] > -1) {
	 var thisResized = resizeHandle;
	 if (resizeHandle) {
	   var hid = id + "-rs" + dir + "-" + di;
	   var handle = WT.getElement(hid);
	   if (!handle) {
	     DC.resizeHandles[di] = hid;
	     handle = document.createElement('div');
	     handle.setAttribute('id', hid);
	     handle.di = di;
	     handle.style.position='absolute';
	     handle.style[OC.left] = OC.margins[MARGIN_LEFT] + 'px';
	     handle.style[DC.size] = DC.margins[SPACING] + 'px';
	     if (OC.cSize)
	       handle.style[OC.size] = (OC.cSize - OC.margins[MARGIN_RIGHT] -
					OC.margins[MARGIN_LEFT]) + 'px';
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
	 } else {
	   if (DC.resizeHandles[di]) {
	     var handle = WT.getElement(DC.resizeHandles[di]);
	     handle.parentNode.removeChild(handle);
	     DC.resizeHandles[di] = undefined;
	   }
	 }

	 resizeHandle = DC.config[di][RESIZABLE] !== 0;

	 if (first)
	   first = false;
	 else
	   left += DC.margins[SPACING];
       } else {
	 if (DC.resizeHandles[di]) {
	   var handle = WT.getElement(DC.resizeHandles[di]);
	   handle.parentNode.removeChild(handle);
	   DC.resizeHandles[di] = undefined;
	 }
       }

       for (oi = 0; oi < otherCount; ++oi) {
	 var item = DC.getItem(di, oi);
	 if (item && item.w) {
	   var w = item.w;

	   var ts = Math.max(targetSize[di], 0);
	   if (item.span) {
	     var si;

	     var rs = resizeHandle;
	     for (si = 1; si < item.span[dir]; ++si) {
	       if (di + si >= targetSize.length)
		 break;

	       if (rs)
		 ts += RESIZE_HANDLE_MARGIN * 2;

	       rs = DC.config[di + si][RESIZABLE] !== 0;

	       if (targetSize[di + si - 1] > -1 && targetSize[di + si] > -1)
		 ts += DC.margins[SPACING];
	       ts += targetSize[di + si];
	     }
	   }
	   
	   var off;

	   setCss(w, 'visibility', '');

	   var alignment = (item.align >> DC.alignBits) & 0xF;
	   var ps = item.ps[dir];

	   if (ts < ps)
	     alignment = 0;

	   if (!alignment) {
	     var m = item.margin[dir];

	     var tsm = Math.max(0, ts - m);

	     /*
	      * We need to force setting a size if there is a scrollbar
	      * (reasons unknown, only for a vertical scrollbar?)
	      */
	     var setSize = dir == 0 && item.sc[dir];

	     var hidden = isHidden(w);

	     if (!hidden && (setSize || ts != ps || item.layout)) {
	       if (setCss(w, DC.size, tsm + 'px')) {
		 /*
		  * Setting a size no longer cancels the built-in margin!
		  */
		 setItemDirty(item, 1);
		 item.set[dir] = true;
	       }
	     } else {
	       if (!item.fs[dir]) {
		 if (setCss(w, DC.size, ''))
		   setItemDirty(item, 1);
		 if (item.set)
		   item.set[dir] = false;
	       } else if (dir == HORIZONTAL)
		 setCss(w, DC.size, (item.fs[dir] - m) + 'px');
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

	     ps -= item.margin[dir];

	     if (item.layout) {
	       if (setCss(w, DC.size, ps + 'px'))
		 setItemDirty(item, 1);
	       item.set[dir] = true;
	     } else if (ts >= ps && item.set[dir]) {
	       if (setCss(w, DC.size, ps + 'px'))
		 setItemDirty(item, 1);
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
			  item.set[HORIZONTAL]
			  ? Math.round(item.size[HORIZONTAL]) : -1,
			  item.set[VERTICAL]
			  ? Math.round(item.size[VERTICAL]) : -1,
			  true);

	     item.dirty = 0;
	   }
	 }
       }

       if (targetSize[di] > -1)
	 left += targetSize[di];
     }

     if (DC.resizeHandles.length > dirCount) {
       for (var i=dirCount; i < DC.resizeHandles.length; i++) {
	 if (DC.resizeHandles[i]) {
	   var handle = WT.getElement(DC.resizeHandles[i]);
	   handle.parentNode.removeChild(handle);
	 }
       }
       DC.resizeHandles.length = dirCount;
     }

     $(widget).children("." + OC.handleClass)
       .css(DC.size,
	    (cSize - DC.margins[MARGIN_RIGHT] - DC.margins[MARGIN_LEFT])
	    + 'px');
   }

   this.setConfig = function(conf) {
     /*
      * Flush preferred size measurements
      */
     var oldConfig = config;
     config = conf;

     DirConfig[0].config = config.cols;
     DirConfig[1].config = config.rows;

     DirConfig[0].stretched = [];
     DirConfig[1].stretched = [];

     var i, il, childLayouts = {};
     for (i = 0, il = oldConfig.items.length; i < il; ++i) {
       var oldItem = oldConfig.items[i];

       if (oldItem) {
	 var newItem = getItem(oldItem.id);

	 if (newItem) {
	   newItem.ps = oldItem.ps;
	   newItem.sc = oldItem.sc;
	   newItem.ms = oldItem.ms;
	   newItem.size = oldItem.size;
	   newItem.psize = oldItem.psize;
	   newItem.fs = oldItem.fs;
	   newItem.margin = oldItem.margin;
	   newItem.set = oldItem.set;
	 } else {
	   if (oldItem.set) {
	     if (oldItem.set[HORIZONTAL])
	       setCss(oldItem.w, DirConfig[HORIZONTAL].size, '');
	     if (oldItem.set[VERTICAL])
	       setCss(oldItem.w, DirConfig[VERTICAL].size, '');
	   }
	 }
       }
     }

     layoutDirty = true;
     itemDirty = true;

     APP.layouts2.scheduleAdjust();
   };

   this.getId = function() {
     return id;
   };

   this.setElDirty = function(el) {
     var item = getItem(el.id);
     if (item) {
       item.dirty = 2;
       itemDirty = true;
       APP.layouts2.scheduleAdjust();
     }
   };

   this.setItemsDirty = function(items) {
     var i, il;

     var colCount = DirConfig[HORIZONTAL].config.length;
     for (i = 0, il = items.length; i < il; ++i) {
       var row = items[i][0], col = items[i][1];
       var item = config.items[row * colCount + col];

       if (item) {
	 item.dirty = 2;
	 /*
	  * When the item contains a layout, a change may also impact this:
	  * it could become a different layout (e.g. WStackedWidget) or
	  * a hidden layout (e.g. WPanel). Assume in general that it could
	  * be the case that we need to reset the layout-based size, but don't
	  * do it yet since it causes flicker when nothing drastic changed.
	  */
	 if (item.layout) {
	   item.layout = false;
	   item.wasLayout = true;

	   APP.layouts2.setChildLayoutsDirty(self, item.w);
	 }
       }
     }

     itemDirty = true;
   };

   this.setDirty = function() {
     layoutDirty = true;
   };

   this.setAllDirty = function() {
     var i, il;
     for (i = 0, il = config.items.length; i < il; ++i) {
       var item = config.items[i];
       if (item)
	 item.dirty = 2;
     }

     itemDirty = true;
   };

   this.setChildSize = function(widget, dir, preferredSize) {
     var colCount = DirConfig[HORIZONTAL].config.length,
         DC = DirConfig[dir],
         i, il;

     var item = getItem(widget.id);
     if (item) {
       var di = (dir === HORIZONTAL ? i % colCount : i / colCount);
       var alignment = (item.align >> DC.alignBits) & 0xF;

       if (alignment || !DC.stretched[di]) {
	 if (!item.ps)
	   item.ps = [];
	 item.ps[dir] = preferredSize;
       }

       item.layout = true;

       setItemDirty(item, 1);
     }
   };

   function initializeParent() {
       var widget = WT.getElement(id);

       topLevel = parentId == null;
       parent = null;
       parentItemWidget = null;
       propagateSizeToParent = true;
       parentInitialized = true;
       parentMargin = [];
       parentWithWtPS = false;

       if (!topLevel) {
	 parent = jQuery.data(document.getElementById(parentId), 'layout');
	 parentItemWidget = widget;
	 parentMargin[HORIZONTAL] = boxMargin(parentItemWidget, HORIZONTAL);
	 parentMargin[VERTICAL] = boxMargin(parentItemWidget, VERTICAL);
       } else {
	 /*
	  * While we are a single child in a parent, or if we have a
	  * wtGetPS() function on that parent, we can go further up
	  * looking for an ancestor layout
	  */
	 var c = widget, p = c.parentNode;

	 parentMargin = [0, 0];
	 for (;p != document;) {
	   parentMargin[HORIZONTAL] += boxMargin(p, HORIZONTAL);
	   parentMargin[VERTICAL] += boxMargin(p, VERTICAL);

	   if (p.wtGetPS)
	     parentWithWtPS = true;

	   var l = jQuery.data(p.parentNode, 'layout');
	   if (l) {
	     parentItemWidget = p;
	     parent = l;
	     break;
	   }

	   c = p;
	   p = c.parentNode;

	   if (p.childNodes.length != 1 && !p.wtGetPS)
             propagateSizeToParent = false;
	 }

	 var container = widget.parentNode;
	 for (var i = 0; i < 2; ++i) {
	    var sizedByAnimation = i == 1 &&
	       typeof container.nativeHeight !== 'undefined';
	   if (sizedByAnimation)
	     DirConfig[i].sizeSet = WT.parsePx(container.nativeHeight) != 0;
	   else
	     DirConfig[i].sizeSet = WT.pxself(container, DirConfig[i].size)
	       != 0;
	 }
       }
   }

   this.measure = function(dir) {
     var widget = WT.getElement(id);

     if (!widget)
       return;

     if (WT.isHidden(widget))
       return;

     if (!parentInitialized)
       initializeParent();

     if (itemDirty || layoutDirty) {
       var container = topLevel ? widget.parentNode : null;
       measure(dir, widget, container);
     }

     if (dir == VERTICAL)
       itemDirty = layoutDirty = false;
   };

   this.setMaxSize = function(width, height) {
     DirConfig[HORIZONTAL].maxSize = width;
     DirConfig[VERTICAL].maxSize = height;
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

   this.WT = WT;
 });

WT_DECLARE_APP_MEMBER
(1, JavaScriptObject, "layouts2",
  new (function() {
    var topLevelLayouts = [], adjusting = false;
    var self = this;
    var measureVertical = false;

    this.find = function(id) {
      var el = document.getElementById(id);
      return el ? jQuery.data(el, 'layout') : null;
    };

    this.setDirty = function(id) {
      var layout = this.find(id);
      if (layout) {
	layout.setDirty();
	self.scheduleAdjust();
      }
    };

    this.setElementDirty = function(el) {
      /*
       * We need to find the first parent of el that is a layout item, and
       * then mark it dirty.
       */
      var item = el;
      el = el.parentNode;
      while (el && el != document.body) {
	var layout = jQuery.data(el, 'layout');
	if (layout)
	  layout.setElDirty(item);
	item = el;
	el = el.parentNode;
      }
    };

    this.setChildLayoutsDirty = function(layout, itemWidget) {
      var i, il;

      for (i=0, il = layout.descendants.length; i < il; ++i) {
	var ll = layout.descendants[i];

	if (itemWidget) {
	  var lw = layout.WT.getElement(ll.getId());
	  if (lw && !layout.WT.contains(itemWidget, lw))
	    continue;
	}

	ll.setDirty();
      }
    };

    this.add = function(layout) {
      function addIn(layouts, layout) {
	var i, il;

	for (i=0, il = layouts.length; i < il; ++i) {
	  var ll = layouts[i];

	  if (ll.getId() == layout.getId()) {
	    /*
	     * Is being re-added (nested layout of item removed + added
	     * in same event)
	     */
	    layouts[i] = layout;
	    layout.descendants = ll.descendants;
	    return;
	  } else if (ll.contains(layout)) {
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

      self.scheduleAdjust();
    };

    var adjustScheduled = false, adjustLoop = 0;

    this.scheduleAdjust = function(forceMeasureVertical) {
      if (forceMeasureVertical)
	measureVertical = true;

      if (adjustScheduled)
	return;

      if (adjusting)
	++adjustLoop;
      else
	adjustLoop = 0;

      if (adjustLoop >= 6)
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
	  else if (dir == HORIZONTAL)
	    ll.setAllDirty();

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

    this.adjustNow = function() {
      if (adjustScheduled)
	self.adjust();
    };

    var resizeDelay = null;
    
    window.onresize = function() {
      clearTimeout(resizeDelay);
      resizeDelay = setTimeout(function() {
	  resizeDelay = null;
	  self.scheduleAdjust(true);
	}, 20);
    };

    window.onshow = function() {
      measureVertical = true;
      self.adjust();
    };

  }) ()
);
