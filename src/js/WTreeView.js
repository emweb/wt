/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER
(1, "WTreeView",
 function(APP, el, contentsContainer, headerContainer, column1Fixed) {
   jQuery.data(el, 'obj', this);

   var contents = contentsContainer.firstChild;
   var headers = headerContainer.firstChild;

   var self = this;
   var WT = APP.WT;

   function getItem(event) {
     var columnId = -1, nodeId = null, selected = false,
         drop = false, ele = null;

     var t = WT.target(event);

     while (t) {
       if (t.className.indexOf('c1 rh') == 0) {
	 if (columnId == -1)
           columnId = 0;
       } else if (t.className.indexOf('Wt-tv-c') == 0) {
	 if (t.className.indexOf('Wt-tv-c') == 0)
           columnId = t.className.split(' ')[0].substring(7) * 1;
	 else if (columnId == -1)
	 columnId = 0;
	 if (t.getAttribute('drop') === 'true')
	   drop = true;
	 ele = t;
       } else if (t.className == 'Wt-tv-node') {
	 nodeId = t.id;
	 break;
       }
       if (t.className === 'Wt-selected')
	 selected = true;
       t = t.parentNode;
       if (WT.hasTag(t, 'BODY'))
	 break;
     }

     return { columnId: columnId, nodeId: nodeId, selected: selected,
	      drop: drop, el: ele };
   };

   this.click = function(obj, event) {
     var item = getItem(event);
     if (item.columnId != -1) {
       APP.emit(el, { name: 'itemEvent', eventObject: obj, event: event },
		item.nodeId, item.columnId, 'clicked', '', '');
     }
   };

   this.dblClick = function(obj, event) {
     var item = getItem(event);
     if (item.columnId != -1) {
       APP.emit(el, { name: 'itemEvent', eventObject: obj, event: event },
		item.nodeId, item.columnId, 'dblclicked', '', '');
     }
   };

   this.mouseDown = function(obj, event) {
     WT.capture(null);
     var item = getItem(event);
     if (item.columnId != -1) {
       APP.emit(el, { name: 'itemEvent', eventObject: obj, event: event },
		item.nodeId, item.columnId, 'mousedown', '', '');
       if (el.getAttribute('drag') === 'true' && item.selected)
         APP._p_.dragStart(el, event);
     }
   };

   this.mouseUp = function(obj, event) {
     var item = getItem(event);
     if (item.columnId != -1) {
       APP.emit(el, { name: 'itemEvent', eventObject: obj, event: event },
		item.nodeId, item.columnId, 'mouseup', '', '');
     }
   };

   this.resizeHandleMDown = function(obj, event) {
     var parent = obj.parentNode.parentNode,
         c = parent.className.split(' ')[0];

     if (c) {
       var r = WT.getCssRule('#' + el.id + ' .' + c),
           cw = WT.pxself(r, 'width'),
           minDelta = -cw,
           maxDelta = 10000;

       new WT.SizeHandle(WT, 'h', obj.offsetWidth, el.offsetHeight,
	                 minDelta, maxDelta, 'Wt-hsh',
			 function (delta) {
			   var newWidth = cw + delta,
			       columnId = c.substring(7) * 1;
			   r.style.width = newWidth + 'px';
			   self.adjustColumns();
			     APP.emit(el, 'columnResized', columnId,
				      parseInt(newWidth));
			 }, obj, el, event, -2, -1);
     }
   };

   /*
    * this adjusts invariants that take into account column resizes
    *
    * c0w is set as soon as possible.
    *
    *  if (!column1 fixed):
    *    1) width('Wt-headerdiv') = sum(column widths)
    *    2) width('float: right') = sum(column(-1) widths)
    *    3) width(table parent) = width('Wt-headerdiv')
    *  else
    *    4) width('Wt-rowc') = sum(column(-1) widths)
    */
   this.adjustColumns = function() {
     var t = contents.firstChild, // table parent
         hc = headers.firstChild, // Wt-tv-row
         allw_1=0, allw=0,
         c0id = headers.lastChild.className.split(' ')[0],
         c0r = WT.getCssRule('#' + el.id + ' .' + c0id);

     if (column1Fixed)
       hc = hc.firstChild; // Wt-tv-rowc

     if (WT.isHidden(el))
       return;

     for (var i=0, length=hc.childNodes.length; i < length; ++i) {
       if (hc.childNodes[i].className) { // IE may have only a text node
	 var cl = hc.childNodes[i].className.split(' ')[0],
	     r = WT.getCssRule('#' + el.id + ' .' + cl);

	 if (r.style.display == 'none')
	   continue;

	 // 7 = 2 * 3px (padding) + 1px border
	 allw_1 += WT.pxself(r, 'width') + 7;
       }
     }

     if (!column1Fixed)
       if (!c0r.style.width)  // first resize and c0 width not set
	 c0r.style.width = (headers.offsetWidth - hc.offsetWidth - 8) + 'px';
       else
	 $(el).find('.Wt-headerdiv .' + c0id).css('width', c0r.style.width);

     allw = allw_1 + WT.pxself(c0r, 'width') + (WT.isIE6 ? 10 : 8);

     if (!column1Fixed) {
       headers.style.width = t.style.width = allw + 'px';
       hc.style.width = allw_1 + 'px';
     } else {
       var r = WT.getCssRule('#' + el.id + ' .Wt-tv-rowc');
       r.style.width = allw_1 + 'px';
       $(el).find('.Wt-tv-rowc').css('width', allw_1 + 'px').css('width', '');
       el.changed = true;
       this.autoJavaScript();
     }
   };

   var dropEl = null;

   el.handleDragDrop=function(action, object, event, sourceId, mimeType) {
     if (dropEl) {
       dropEl.className = dropEl.classNameOrig;
       dropEl = null;
     }

     if (action=='end')
       return;

     var item = getItem(event);

     if (!item.selected && item.drop && item.columnId != -1) {
       if (action=='drop') {
	 APP.emit(el, { name: 'itemEvent', eventObject: object, event: event },
		  item.nodeId, item.columnId, 'drop', sourceId, mimeType);
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

  /*
   * This adjusts invariants that depend on the size of the whole
   * treeview:
   *
   *  - changes to the total width (tw inc. affected by scrollbar)
   *  - when column1 is fixed:
   *    * .row width
   *    * table parent width
   */
  this.autoJavaScript = function() {
    if (el.parentNode == null) {
      el = contentsContainer = headerContainer = contents = headers = null;
      this.autoJavaScript = function() { };
      return;
    }

    if (WT.isHidden(el))
      return;

    var $el=$(el),
      tw = $el.innerWidth(),
        c0id, c0r, c0w = null;

    var scrollwidth = contentsContainer.offsetWidth
      - contentsContainer.clientWidth;

    tw -= scrollwidth;

    if ($el.hasClass('column1')) {
      c0id = $el.find('.Wt-headerdiv').get(0).lastChild.className.split(' ')[0];
      c0r = WT.getCssRule('#' + el.id + ' .' + c0id);
      c0w = WT.pxself(c0r, 'width');
    }

    // XXX: IE's incremental rendering foobars completely
    if ((!WT.isIE || tw > 100)
        && (tw != contentsContainer.tw ||
            c0w != contentsContainer.c0w ||
            el.changed)) {
      var adjustColumns = !el.changed;

      contentsContainer.tw = tw;
      contentsContainer.c0w = c0w;

      c0id = $el.find('.Wt-headerdiv').get(0).lastChild.className.split(' ')[0];
      c0r = WT.getCssRule('#' + el.id + ' .' + c0id);

      var table = contents.firstChild,
          r = WT.getCssRule('#' + el.id + ' .cwidth'),
          contentstoo = (r.style.width == headers.style.width),
          hc = headers.firstChild;

      r.style.width = tw + 'px';
      contentsContainer.style.width = (tw + scrollwidth) + 'px';
      headers.style.width = table.offsetWidth + 'px';

      if (c0w != null) {
        var w = tw - c0w - (WT.isIE6 ? 10 : 8);

        if (w > 0) {
          var w2 = Math.min(w,
	      WT.pxself(WT.getCssRule('#' + el.id + ' .Wt-tv-rowc'),'width'));
          tw -= (w - w2);

          WT.getCssRule('#' + el.id + ' .Wt-tv-row').style.width = w2 + 'px';
          $el.find(' .Wt-tv-row').css('width', w2 + 'px').css('width', '');
          headers.style.width=tw + 'px';
          table.style.width=tw + 'px';
        }
      } else if (contentstoo) {
        headers.style.width=r.style.width;
        table.style.width=r.style.width;
      }

      if (!column1Fixed)
	c0r.style.width = (table.offsetWidth - hc.offsetWidth - 8) + 'px';

      el.changed = false;

      if (adjustColumns/* && WT.isIE */)
	self.adjustColumns();
    }
  };

  self.adjustColumns();

 });
