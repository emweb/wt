/*
 * Copyright (C) 2011 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER
(1, JavaScriptConstructor, "Resizable",
 function(WT, el) {
   var handler = null, downXY = null,
     iwidth, iheight, /* initial CSS width and height */
     cwidth, cheight, /* initial client width and height */
     minwidth, minheight;

   var cssMinWidth = WT.css(el, 'minWidth'),
       cssMinHeight = WT.css(el, 'minHeight');

   if (WT.isIE6) {
     /*
      * IE6 does not support min-width, min-height, but still provides them
      * in the cssText
      */
     function fishCssText(el, property) {
       var m = new RegExp(property + ":\\s*(\\d+(?:\\.\\d+)?)\\s*px", "i")
	 .exec(el.style.cssText);
       return (m && m.length == 2) ? m[1] + 'px' : '';
     }

     cssMinWidth = fishCssText(el, 'min-width');
     cssMinHeight = fishCssText(el, 'min-height');
   }

   if (cssMinWidth == '0px')
     minwidth = el.clientWidth;
   else
     minwidth = WT.parsePx(cssMinWidth);

   if (cssMinHeight == '0px')
     minheight = el.clientHeight;
   else
     minheight = WT.parsePx(cssMinHeight);

   function onMouseMove(event) {
    var xy = WT.pageCoordinates(event);

    var dx = xy.x - downXY.x, dy = xy.y - downXY.y, overflow = 0;

    var vw = Math.max(iwidth + dx, minwidth + (iwidth - cwidth));
    el.style.width = vw + 'px';

    var vh = Math.max(iheight + dy, minheight + (iheight - cheight));
    el.style.height = vh + 'px';

    if (handler)
      handler(vw, vh);
  }

  function onMouseUp(event) {
    el.onmousemove = null;
    el.onmouseup = null;

    if (handler)
      handler(WT.pxself(el, 'width'), WT.pxself(el, 'height'), true);
  }

  function onMouseDown(event) {
    var xy = WT.widgetCoordinates(el, event);

    if (el.offsetWidth - xy.x < 16 && el.offsetHeight - xy.y < 16) {
      downXY = WT.pageCoordinates(event);
      iwidth = WT.innerWidth(el);
      iheight = WT.innerHeight(el);
      cwidth = el.clientWidth;
      cheight = el.clientHeight;

      WT.capture(null);
      WT.capture(el);

      el.onmousemove = onMouseMove;
      el.onmouseup = onMouseUp;
    }
  }

  $(el).mousedown(onMouseDown);

  this.onresize = function(f) {
    handler = f;
  };
 });
