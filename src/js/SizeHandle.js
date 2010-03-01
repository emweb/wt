/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER
  (1, "SizeHandle",
   function(WT, orientation, width, height, minDelta, maxDelta,
	    dragWidgetClass, doneFn, el, event) {
     var handle = document.createElement('div');
     handle.style.position = 'absolute';
     handle.style.zIndex = '100';

     if (orientation == 'v') {
       handle.style.width = height + 'px';
       handle.style.height = width + 'px';
     } else {
       handle.style.height = height + 'px';
       handle.style.width = width + 'px';
     }

     var offset = WT.widgetCoordinates(el, event);
     var elpos = WT.widgetPageCoordinates(el);
     var mx = WT.px(el, 'marginLeft');
     var my = WT.px(el, 'marginTop');
     elpos.x -= mx;
     elpos.y -= my;
     offset.x += mx;
     offset.y += my;

     handle.style.left = elpos.x + 'px';
     handle.style.top = elpos.y + 'px';
     handle.className = dragWidgetClass;

     document.body.appendChild(handle);

     WT.capture(handle);
     WT.cancelEvent(event);

     function computeDelta(event) {
       var p = WT.pageCoordinates(event), result;
       if (orientation == 'h')
         result = (p.x - offset.x) - elpos.x;
       else
         result = (p.y - offset.y) - elpos.y;

       return Math.min(Math.max(result, minDelta), maxDelta);
     }

     handle.onmousemove = function(event) {
       var delta = computeDelta(event);
       if (orientation == 'h')
         handle.style.left = (elpos.x + delta) + 'px';
       else
         handle.style.top = (elpos.y + delta) + 'px';
     };

     handle.onmouseup = function(event) {
       if (handle.parentNode != null) {
	 handle.parentNode.removeChild(handle);
	 doneFn(computeDelta(event));
       }
     };
  }
   );