/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER
(1, JavaScriptConstructor, "WTableView",
 function(APP, el, contentsContainer, initialScrollTop, headerContainer, headerColumnsContainer,
      selectedClass) {
   el.wtObj = this;

   var self = this;
   var WT = APP.WT;
   var rtl = $(document.body).hasClass('Wt-rtl');

   /** @const */ var EnsureVisible = 0;
   /** @const */ var PositionAtTop = 1;
   /** @const */ var PositionAtBottom = 2;
   /** @const */ var PositionAtCenter = 3;

   function rtlScrollLeft(o) {
     if (rtl) {
       if (WT.isGecko)
     return -o.scrollLeft;
       else
     return o.scrollWidth - o.clientWidth - o.scrollLeft;
     } else
       return o.scrollLeft;
   }

   var scrollX1 = 0, scrollX2 = 0, scrollY1 = 0, scrollY2 = 0;
   var scrollToPendingCount = 0;
   var initialScrollTopSet = initialScrollTop === 0;

   /*
    * We need to remember this for when going through a hide()
    * show() cycle.
    */
   var scrollTop = 0, scrollLeft = 0, currentWidth = 0, currentHeight = 0;

   function maybeEmitScrolled() {
     if (contentsContainer.clientWidth && contentsContainer.clientHeight
         && (scrollToPendingCount === 0)
         && (contentsContainer.scrollTop < scrollY1
     || contentsContainer.scrollTop > scrollY2
     || contentsContainer.scrollLeft < scrollX1
     || contentsContainer.scrollLeft > scrollX2)) {
       APP.emit(el, 'scrolled',
        Math.round(rtlScrollLeft(contentsContainer)),
            Math.round(contentsContainer.scrollTop),
        Math.round(contentsContainer.clientWidth),
            Math.round(contentsContainer.clientHeight));
     }
   }

   this.onContentsContainerScroll = function() {
     scrollLeft = headerContainer.scrollLeft
           = contentsContainer.scrollLeft;
     scrollTop = headerColumnsContainer.scrollTop
            = contentsContainer.scrollTop;
     maybeEmitScrolled();
   };

   contentsContainer.wtResize = function(o, w, h, setSize) {
     if (!initialScrollTopSet) {
       o.scrollTop = initialScrollTop;
       o.onscroll();
       initialScrollTopSet = true;
     }
     if ((w - currentWidth) > (scrollX2 - scrollX1)/2 ||
         (h - currentHeight) > (scrollY2 - scrollY1)/2) {
       currentWidth = w; currentHeight = h;
       var height = o.clientHeight == o.firstChild.offsetHeight
         ? -1
         : o.clientHeight;
       APP.emit(el, 'scrolled',
        Math.round(rtlScrollLeft(o)),
        Math.round(o.scrollTop),
        Math.round(o.clientWidth),
        Math.round(height));
     }
   };

   function isSelected(item) {
     var $t = $(item.el);
     return $t.hasClass(selectedClass);
   }

   function getItem(event) {
     var columnId = -1, rowIdx = -1, selected = false,
         drop = false, ele = null;

     var t = WT.target(event);

     while (t) {
       var $t = $(t);
       if ($t.hasClass('Wt-tv-contents')) {
     break;
       } else if ($t.hasClass('Wt-tv-c')) {
     if (t.getAttribute('drop') === 'true')
       drop = true;
     if ($t.hasClass(selectedClass))
       selected = true;
     ele = t;
     t = t.parentNode;
     columnId = t.className.split(' ')[0].substring(7) * 1;
     rowIdx = $t.index();
     break;
       }
       t = t.parentNode;
     }

     return { columnId: columnId, rowIdx: rowIdx, selected: selected,
          drop: drop, el: ele };
   };

   function rowHeight() {
     return WT.pxself(contentsContainer.firstChild, "lineHeight");
   }

   function indexOf(child) {
     var i, il, plist = child.parentNode.childNodes;

     for (i = 0, il = plist.length; i < il; ++i)
       if (plist[i] == child)
     return i;

     return -1;
   }

   function resizeColumn(header, delta) {
     var rtl = $(document.body).hasClass('Wt-rtl');

     if (rtl)
       delta = -delta;

     var columnClass = header.className.split(' ')[0],
         columnId = columnClass.substring(7) * 1,
     headers = header.parentNode,
     headerColumn = headers.parentNode !== headerContainer,
     contents = headerColumn
              ? headerColumnsContainer.firstChild
              : contentsContainer.firstChild,
     wt_tv_contents = contents.firstChild,
     column = $(contents).find('.' + columnClass).get(0),
     h = header.nextSibling, c = column.nextSibling,
         newWidth = WT.pxself(header, 'width') - 1 + delta;

     var cwidth = (WT.pxself(headers, 'width') + delta) + 'px';

     headers.style.width
       = contents.style.width
       = wt_tv_contents.style.width
       = cwidth;

     if (headerColumn) {
       headerColumnsContainer.style.width = cwidth;
       headerColumnsContainer.firstChild.style.width = cwidth;
       contentsContainer.style.left = cwidth;
       headerContainer.style.left = cwidth;
     }

     header.style.width = (newWidth + 1) + 'px';
     column.style.width = (newWidth + 7) + 'px';
     APP.layouts2.adjust(el.childNodes[0].id, [[1,1]]);

     for (; h; h = h.nextSibling) {
       if (c) {
     if (!rtl)
       c.style.left = (WT.pxself(c, 'left') + delta) + 'px';
     else
       c.style.right = (WT.pxself(c, 'right') + delta) + 'px';
     c = c.nextSibling;
       }
     }

     APP.emit(el, 'columnResized', columnId, parseInt(newWidth));
     self.autoJavaScript();
   }

   var startDrag = null;

   this.mouseDown = function(obj, event) {
     WT.capture(null);

     var item = getItem(event);

     if (!event.ctrlKey && !event.shiftKey) {
       /*
    * For IE, there is only global event object which does not survive
    * the event lifetime
    */
       var e = {
         ctrlKey: event.ctrlKey,
     shiftKey: event.shiftKey,
     target: event.target,
     srcElement: event.srcElement,
     type: event.type,
     which: event.which,
     touches: event.touches,
     changedTouches: event.changedTouches,
     pageX: event.pageX,
     pageY: event.pageY,
     clientX: event.clientX,
     clientY: event.clientY
       };

       startDrag = setTimeout(function() {
     if (el.getAttribute('drag') === 'true' && isSelected(item)) {
       APP._p_.dragStart(el, e);
     }
       }, 400);
     }
   };

   this.mouseUp = function(obj, event) {
     clearTimeout(startDrag);
   };

   this.resizeHandleMDown = function(obj, event) {
     var header = obj.parentNode,
         cw = WT.pxself(header, 'width') - 1,
         minDelta = -cw,
         maxDelta = 10000;

     var rtl = $(document.body).hasClass('Wt-rtl');
     if (rtl) {
       var tmp = minDelta;
       minDelta = -maxDelta;
       maxDelta = -tmp;
     }

     new WT.SizeHandle(WT, 'h', obj.offsetWidth, el.offsetHeight,
               minDelta, maxDelta, 'Wt-hsh2',
               function (delta) {
             resizeColumn(header, delta);
               }, obj, el, event, -2, -1);
   };

   var touchStartTimer;

   var touches = 0;
   this.touchStart = function(obj, event) {

     var item = getItem(event);

     if (!event.ctrlKey && !event.shiftKey) {
       /*
    * For IE, there is only global event object which does not survive
    * the event lifetime
    */
       var e = {
         ctrlKey: event.ctrlKey,
     shiftKey: event.shiftKey,
     target: event.target,
     srcElement: event.srcElement,
     type: event.type,
     which: event.which,
     touches: event.touches,
     changedTouches: event.changedTouches,
     pageX: event.pageX,
     pageY: event.pageY,
     clientX: event.clientX,
     clientY: event.clientY
       };

     }
     if (event.touches.length > 1) {
       clearTimeout(touchStartTimer);
       touchStartTimer = setTimeout(function(){emitTouchSelect(obj, event);}, 1000);
       touches = event.touches.length;
     }
     else{
       clearTimeout(touchStartTimer);
       touchStartTimer = setTimeout(function(){emitTouchSelect(obj, event);}, 50);
       touches = 1;
     }
   };

   function emitTouchSelect(obj, event) {
     APP.emit(el, { name: 'itemTouchSelectEvent', eventObject: obj, event: event});
   };

   this.touchMove = function(obj, event) {
     if (event.touches.length == 1 && touchStartTimer)
       clearTimeout(touchStartTimer);
   };

   this.touchEnd = function(obj, event) {
     if (touchStartTimer && touches != 1)
       clearTimeout(touchStartTimer);
   };

   this.scrolled = function(X1, X2, Y1, Y2) {
     scrollX1 = X1;
     scrollX2 = X2;
     scrollY1 = Y1;
     scrollY2 = Y2;
   };

   this.resetScroll = function() {
     headerContainer.scrollLeft = scrollLeft;
     contentsContainer.scrollLeft = scrollLeft;
     contentsContainer.scrollTop = scrollTop;
     headerColumnsContainer.scrollTop = scrollTop;
   };

   this.setScrollToPending = function() {
     scrollToPendingCount += 1;
   };

   this.scrollToPx = function(x, y) {
     scrollTop = y;
     scrollLeft = x;
     this.resetScroll();
   };

   this.scrollTo = function(x, y, hint) {
     if (scrollToPendingCount > 0)
       scrollToPendingCount -= 1;
     if (y != -1) {
       var top = contentsContainer.scrollTop,
       height = contentsContainer.clientHeight;
       if (hint == EnsureVisible) {
     if (top + height < y)
       hint = PositionAtTop;
     else if (y < top)
       hint = PositionAtBottom;
       }

       switch (hint) {
       case PositionAtTop:
         contentsContainer.scrollTop = y; break;
       case PositionAtBottom:
         contentsContainer.scrollTop = y - (height - rowHeight()); break;
       case PositionAtCenter:
         contentsContainer.scrollTop = y - (height - rowHeight())/2; break;
       }

       contentsContainer.onscroll();
     }
   };

   var dropEl = null;

   el.handleDragDrop = function(action, object, event, sourceId, mimeType) {
     if (dropEl) {
       dropEl.className = dropEl.classNameOrig;
       dropEl = null;
     }

     if (action == 'end')
       return;

     var item = getItem(event);

     if (!item.selected && item.drop) {
       if (action == 'drop') {
     APP.emit(el, { name: 'dropEvent', eventObject: object, event: event },
          item.rowIdx, item.columnId, sourceId, mimeType);
       } else {
         object.className = 'Wt-valid-drop';
         dropEl = item.el;
         dropEl.classNameOrig = dropEl.className;
         dropEl.className = dropEl.className + ' Wt-drop-site';
       }
     } else {
       object.className = '';
     }
   };

   /* Handle TAB/SHIFT-TAB for cycling through editors in the right order */
   this.onkeydown = function(e) {
     var event = e||window.event;

     var leftKey = 37,
       upKey = 38,
       rightKey = 39,
       downKey = 40;

     if (event.keyCode == 9) {
       WT.cancelEvent(event);

       /* Find next/prev input element, first by row, then by column */
       var item = getItem(event);
       if (!item.el)
     return;

       var col = item.el.parentNode,
           rowi = indexOf(item.el),
           coli = indexOf(col),
           cols = col.parentNode.childNodes.length,
           rows = col.childNodes.length,
       back = event.shiftKey,
       wrapped = false;

       var i = rowi, j;

       for (;;) {
     for (;
          back ? i >= 0 : i < rows;
          i = (back ? i-1 : i+1)) {
       if (i == rowi && !wrapped)
         j = back ? coli - 1 : coli + 1;
       else
         j = back ? cols - 1 : 0;
       for (;
        back ? j >= 0 : j < cols;
        j = (back ? j-1 : j+1)) {
         /* We have wrapped and arrived back at the beginning */
         if (i == rowi && j == coli)
           return;
         col = col.parentNode.childNodes[j];
         var elij = col.childNodes[i];
         var inputs = $(elij).find(":input");
         if (inputs.size() > 0) {
           setTimeout(function() { inputs.focus(); inputs.select();}, 0);
           return;
         }
       }
     }
     i = back ? rows - 1 : 0;
     wrapped = true;
       }
     }

     /* If keycode is up/down/right/left */
     else if (event.keyCode >= leftKey && event.keyCode <= downKey) {
       var currentEl = WT.target(event);

       function isInput(el) {
     return (WT.hasTag(el,'INPUT') && el.type == 'text')
       || WT.hasTag(el, 'TEXTAREA');
       }

       // do not allow arrow navigation from select
       if (WT.hasTag(currentEl, 'SELECT'))
     return;

       var item = getItem(event);
       if (!item.el)
     return;

       var col = item.el.parentNode,
           rowi = indexOf(item.el),
           coli = indexOf(col),
           cols = col.parentNode.childNodes.length,
       rows = col.childNodes.length;

       switch (event.keyCode) {
     case rightKey:
     if (isInput(currentEl)) {
         var range = WT.getSelectionRange(currentEl);
         if (range.start != currentEl.value.length)
           return;
       }
       coli++; break;
     case upKey:
       rowi--; break;
     case leftKey:
       if (isInput(currentEl)) {
         var range = WT.getSelectionRange(currentEl);
         if (range.start != 0)
           return;
       }
       coli--; break;
     case downKey:
       rowi++; break;
     default:
       return;
       }

       WT.cancelEvent(event);

       if (rowi > -1 && rowi < rows && coli > -1 && coli < cols) {
     col = col.parentNode.childNodes[coli];
     var elToSelect = col.childNodes[rowi];
     var inputs = $(elToSelect).find(":input");
     if (inputs.size() > 0) {
       setTimeout(function() { inputs.focus(); }, 0);
       return;
     }
       }
     }
   };

   this.autoJavaScript = function() {
     if (el.parentNode == null) {
       el = contentsContainer = headerContainer = null;
       this.autoJavaScript = function() { };
       return;
     }

     if (WT.isHidden(el))
       return;

     if (!WT.isIE && (scrollTop != contentsContainer.scrollTop
         || scrollLeft != contentsContainer.scrollLeft)) {
       if (typeof scrollLeft === 'undefined') {
     if (rtl && WT.isGecko) {
       headerContainer.scrollLeft = contentsContainer.scrollLeft
         = scrollLeft = 0;
     } else {
       scrollLeft = contentsContainer.scrollLeft;
     }
       } else {
     headerContainer.scrollLeft = contentsContainer.scrollLeft
       = scrollLeft;
       }
       headerColumnsContainer.scrollTop = contentsContainer.scrollTop
     = scrollTop;
     }

     var tw = el.offsetWidth - WT.px(el, 'borderLeftWidth')
          - WT.px(el, 'borderRightWidth');

     var scrollwidth = contentsContainer.offsetWidth
       - contentsContainer.clientWidth;
     tw -= headerColumnsContainer.clientWidth;

     if (tw > 200  // XXX: IE's incremental rendering foobars completely
         && (tw != contentsContainer.tw)) {
       contentsContainer.tw = tw;

       contentsContainer.style.width = tw + 'px';
       headerContainer.style.width = (tw - scrollwidth) + 'px';
     }

     var rtl = $(document.body).hasClass('Wt-rtl');
     if (!rtl)
       headerContainer.style.marginRight = scrollwidth + 'px';
     else
       headerContainer.style.marginLeft = scrollwidth + 'px';

     var scrollheight = contentsContainer.offsetHeight
       - contentsContainer.clientHeight;

     var pns = headerColumnsContainer.style;
     if (pns && (pns.marginBottom !== scrollheight + 'px')) {
       pns.marginBottom = scrollheight + 'px';
       APP.layouts2.adjust(el.childNodes[0].id, [[1,0]]);
     }
   };
 });
