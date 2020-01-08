/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
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

     function mousemove(event) {
       var delta = computeDelta(event);
       if (orientation == 'h')
         handle.style.left = (elpos.x + delta) + 'px';
       else
         handle.style.top = (elpos.y + delta) + 'px';

       WT.cancelEvent(event);
     }
     function mouseup_common(event) {
       if (handle.parentNode != null) {
         handle.parentNode.removeChild(handle);
         doneFn(computeDelta(event));
       }
     }
     if (document.addEventListener) {
       var domRoot = $('.Wt-domRoot')[0];
       if (!domRoot) {
         // widgetset mode
         domRoot = parent;
       }
       var oldPointerEvents = domRoot.style['pointer-events'];
       if (!oldPointerEvents)
         oldPointerEvents = 'all';
       domRoot.style["pointer-events"] = 'none';
       var oldCursorStyle = document.body.style['cursor'];
       if (!oldCursorStyle)
         oldCursorStyle = 'auto';
       if (orientation == 'h')
         document.body.style['cursor'] = 'ew-resize';
       else
         document.body.style['cursor'] = 'ns-resize';
       function mouseup(event) {
         domRoot.style['pointer-events'] = oldPointerEvents;
         document.body.style['cursor'] = oldCursorStyle;
         document.removeEventListener('mousemove', mousemove, {capture: true});
         document.removeEventListener('mouseup', mouseup, {capture: true});
         document.removeEventListener('touchmove', mousemove, {capture: true});
         document.removeEventListener('touchend', mouseup, {capture: true});
         mouseup_common(event);
       }
       document.addEventListener('mousemove', mousemove, {capture: true});
       document.addEventListener('mouseup', mouseup, {capture: true});
       document.addEventListener('touchmove', mousemove, {capture: true});
       document.addEventListener('touchend', mouseup, {capture: true});
     } else {
       handle.onmousemove = parent.ontouchmove = mousemove;
       handle.onmouseup = parent.ontouchend = function(event) {
         parent.ontouchmove = null;
         parent.ontouchend = null;
         mouseup_common(event);
       };
     }
  }
   );
