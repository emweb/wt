/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER
 (1, JavaScriptConstructor, "SizeHandle",
   function(WT, orientation, width, height, minDelta, maxDelta,
	    dragWidgetClass, doneFn, el, parent, event, offsetX, offsetY) {
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

     var offset, elpos = WT.widgetPageCoordinates(el),
         parentpos = WT.widgetPageCoordinates(parent);

     if (event.touches) {
       offset = WT.widgetCoordinates(el, event.touches[0]);
     } else {
       offset = WT.widgetCoordinates(el, event);
       WT.capture(null);
       WT.capture(handle);
     }

     offsetX -= WT.px(el, 'marginLeft');
     offsetY -= WT.px(el, 'marginTop');
     elpos.x += offsetX - parentpos.x;
     elpos.y += offsetY - parentpos.y;
     offset.x -= offsetX - parentpos.x;
     offset.y -= offsetY - parentpos.y;

     handle.style.left = elpos.x + 'px';
     handle.style.top = elpos.y + 'px';
     handle.className = dragWidgetClass;

     parent.appendChild(handle);

     WT.cancelEvent(event);

     function computeDelta(event) {
       var result, p = WT.pageCoordinates(event);

       if (orientation == 'h')
         result = (p.x - offset.x) - elpos.x;
       else
         result = (p.y - offset.y) - elpos.y;

       return Math.min(Math.max(result, minDelta), maxDelta);
     }

     handle.onmousemove = parent.ontouchmove = function(event) {
       var delta = computeDelta(event);
       if (orientation == 'h')
         handle.style.left = (elpos.x + delta) + 'px';
       else
         handle.style.top = (elpos.y + delta) + 'px';

       WT.cancelEvent(event);
     };

     handle.onmouseup = parent.ontouchend = function(event) {
       if (handle.parentNode != null) {
	 handle.parentNode.removeChild(handle);
	 doneFn(computeDelta(event));
	 parent.ontouchmove = null;
       }
     };
  }
   );