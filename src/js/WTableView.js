/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER
(1, "WTableView",
 function(APP, el, contentsContainer, headerContainer) {
   jQuery.data(el, 'obj', this);

   var SCROLLBAR_WIDTH = 19;

   var self = this;
   var WT = APP.WT;

   function getItem(event) {
     var columnId = -1, rowIdx = -1, selected = false,
         drop = false, ele = null;

     var t = event.target || event.srcElement;

     while (t) {
       var $t = $(t);
       if ($t.hasClass('Wt-tv-contents')) {
	 break;
       } else if ($t.hasClass('Wt-tv-c')) {
	 if (t.getAttribute('drop') === 'true')
	   drop = true;
	 if ($t.hasClass('Wt-selected'))
	   selected = true;
	 ele = t;
	 t = t.parentNode;
	 columnId = t.className.split(' ')[0].substring(7) * 1;
	 break;
       }
       t = t.parentNode;
     }

     return { columnId: columnId, rowIdx: rowIdx, selected: selected,
	      drop: drop, el: ele };
   };

   function resizeColumn(header, delta) {
     var columnClass = header.className.split(' ')[0],
         columnId = columnClass.substring(7) * 1,
         headers = headerContainer.firstChild,
         contents = contentsContainer.firstChild,
	 column = $(contents).find('.' + columnClass).get(0),
         h = header.nextSibling, c = column.nextSibling,
         newWidth = WT.pxself(header, 'width') - 1 + delta;

     headers.style.width = contents.style.width
       = (WT.pxself(headers, 'width') + delta) + 'px';
     header.style.width = (newWidth + 1) + 'px';
     column.style.width = (newWidth + 7) + 'px';

     for (; h; h = h.nextSibling) {
       h.style.left = (WT.pxself(h, 'left') + delta) + 'px';
       if (c) {
	 c.style.left = (WT.pxself(c, 'left') + delta) + 'px';
	 c = c.nextSibling;
       }
     }

     APP.emit(el, 'columnResized', columnId, newWidth);
   }

   this.mouseDown = function(obj, event) {
     WT.capture(null);

     var item = getItem(event);
     if (el.getAttribute('drag') === 'true' && item.selected)
       APP._p_.dragStart(el, event);
   };

   this.resizeHandleMDown = function(obj, event) {
     var header = obj.parentNode.parentNode,
         cw = WT.pxself(header, 'width') - 1,
         minDelta = -cw,
         maxDelta = 10000;

     new WT.SizeHandle(WT, 'h', obj.offsetWidth, el.offsetHeight,
		       minDelta, maxDelta, 'Wt-hsh',
		       function (delta) {
			 resizeColumn(header, delta);
		       }, obj, el, event, -2, -1);
   };

   var dropEl = null;

   el.handleDragDrop=function(action, object, event, sourceId, mimeType) {
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

   this.autoJavaScript = function() {
     if (el.parentNode == null) {
       el = contentsContainer = headerContainer = null;
       this.autoJavaScript = function() { };
       return;
     }

     if (WT.isHidden(el))
       return;

     var tw = el.offsetWidth - WT.px(el, 'borderLeftWidth')
	      - WT.px(el, 'borderRightWidth'),
         vscroll = contentsContainer.scrollHeight
		   > contentsContainer.offsetHeight;

     if (tw > 200  // XXX: IE's incremental rendering foobars completely
         && (tw != contentsContainer.tw
	     || vscroll != contentsContainer.vscroll)) {
       contentsContainer.vscroll = vscroll;
       contentsContainer.tw = tw;
       contentsContainer.style.width = tw + 'px';
       headerContainer.style.width
         = (tw - (vscroll ?  SCROLLBAR_WIDTH  : 0)) + 'px';
     }
   };
 });
