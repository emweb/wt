/*
 * Copyright (C) 2016 Emweb bvba, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER
(3, JavaScriptConstructor, "ScrollVisibility",
 function(APP) {
   var self = this, WT = APP.WT;

   function isVisible(w) {
     if (w.style.visibility == 'hidden' ||
         w.style.display == 'none' ||
         $(w).hasClass('out'))
       return false;
     else {
       w = w.parentNode;
       if (w && !WT.hasTag(w, 'BODY'))
         return isVisible(w);
       else
         return true;
     }
   }

   var nbScrollVisibilityElements = 0;
   var scrollVisibilityElements = {};

   /*
   // Only support IntersectionObserver on desktop for now, because Chrome on Android
   // seems to be very late to notify transitions from visible to invisible.
   var hasIntersectionObserver = !WT.isAndroid && 'IntersectionObserver' in window;
   */
   // IntersectionObserver is still in early stages, let's not rely on it
   var hasIntersectionObserver = false; // FIXME: reenable IntersectionObserver when it matures?

   function elementIsInView(el, margin) {
     if (!isVisible(el)) {
       return false;
     }

     var p = WT.widgetPageCoordinates(el);
     var cx = p.x - document.body.scrollLeft - document.documentElement.scrollLeft;
     var cy = p.y - document.body.scrollTop - document.documentElement.scrollTop;
     var winSize = WT.windowSize();

     // Check intersection between viewport rectangle and element
     var x1 = cx;
     var w1 = el.offsetWidth;
     var y1 = cy;
     var h1 = el.offsetHeight;
     var x2 = -margin;
     var w2 = winSize.x + 2 * margin;
     var y2 = -margin;
     var h2 = winSize.y + 2 * margin;
     return x1 + w1 >= x2 &&
            x2 + w2 >= x1 &&
            y1 + h1 >= y2 &&
            y2 + h2 >= y1;
   }

   function visibilityChecker() {
     if (hasIntersectionObserver) {
       // The element may become visible by a change in the visibility
       // or the display property.
       for (var p in scrollVisibilityElements) {
         if (scrollVisibilityElements.hasOwnProperty(p)) {
           var visible = isVisible(scrollVisibilityElements[p].el);
           if (scrollVisibilityElements[p].visibleIfNotHidden &&
               scrollVisibilityElements[p].visible !== visible) {
             scrollVisibilityElements[p].visible = visible;
             APP.emit(scrollVisibilityElements[p].el, 'scrollVisibilityChanged', visible);
           }
         }
       }
     } else {
       for (var p in scrollVisibilityElements) {
         if (scrollVisibilityElements.hasOwnProperty(p)) {
           var visible = elementIsInView(scrollVisibilityElements[p].el,
       				  scrollVisibilityElements[p].margin);
           if (visible !== scrollVisibilityElements[p].visible) {
             scrollVisibilityElements[p].visible = visible;
             APP.emit(scrollVisibilityElements[p].el, 'scrollVisibilityChanged', visible);
           }
         }
       }
     }
   }

   // For IntersectionObserver
   function intersectionCallback(entries) {
     for (var i = 0; i < entries.length; ++i) {
       var entry = entries[i];
       var el = entry.target;
       var elID = el.id;
       var notHidden = isVisible(el);
       if (entry.intersectionRatio > 0 ||
           // FIXME: if the size of the element is 0,
           // it will still be considered visible if
           // top and left are nonzero. However, this
           // will also be zero if the element is in
           // the top left of the page.
           entry.intersectionRect.top !== 0 ||
           entry.intersectionRect.left !== 0) {
         scrollVisibilityElements[elID].visibleIfNotHidden = true;
         if (scrollVisibilityElements[elID].visible !== notHidden) {
           scrollVisibilityElements[elID].visible = notHidden;
           APP.emit(scrollVisibilityElements[elID].el, 'scrollVisibilityChanged', notHidden);
         }
       } else {
         scrollVisibilityElements[elID].visibleIfNotHidden = false;
         if (scrollVisibilityElements[elID].visible) {
           scrollVisibilityElements[elID].visible = false;
           APP.emit(scrollVisibilityElements[elID].el, 'scrollVisibilityChanged', false);
         }
       }
     }
   }

   var mutObserver = null;
   if ('MutationObserver' in window) {
     mutObserver = new MutationObserver(visibilityChecker);
   }

   function enableVisibilityChecker() {
     if (mutObserver) {
       // Visibility can change if:
       //  - the DOM changes
       //  - the window is resized
       //  - some element is scrolled
       mutObserver.observe(document, {childList: true, attributes: true, subtree: true, characterData: true});
       if (!hasIntersectionObserver) {
         window.addEventListener('resize', visibilityChecker, true);
         window.addEventListener('scroll', visibilityChecker, true);
       }
     } else {
       // IE < 11 does not have MutationObserver, so use 100 ms polling instead.
       mutObserver = setInterval(visibilityChecker, 100);
     }
   }

   function disableVisibilityChecker() {
     if (mutObserver) {
       mutObserver.disconnect();
       if (!hasIntersectionObserver) {
         window.removeEventListener('resize', visibilityChecker, {capture:true});
         window.removeEventListener('scroll', visibilityChecker, {capture:true});
       }
     } else {
       // IE < 11 does not have MutationObserver, so use 100 ms polling instead.
       clearInterval(mutObserver);
       mutObserver = null;
     }
   }

   // Entry has three members:
   //  - el: the HTML element that has to be observed
   //  - margin: the scrollVisibilityMargin in pixels
   //  - visible: whether the element was visible previously
   // If IntersectionObserver is enabled, two extra members are added:
   //  - visibleIfNotHidden: indicates whether the element is inside the viewport regardless of display or visiblility property
   //  - observer: the IntersectionObserver for the element
   this.add = function(entry) {
     // If nbScrollVisibilityElements === 0,
     // the checker was disabled and needs to be enabled
     if (nbScrollVisibilityElements === 0)
       enableVisibilityChecker();

     var elID = entry.el.id;
     var alreadyAdded = elID in scrollVisibilityElements;

     // For IntersectionObserver: disconnect old observer
     if (hasIntersectionObserver && alreadyAdded &&
         scrollVisibilityElements[elID].observer) {
       scrollVisibilityElements[elID].observer.disconnect();
     }

     // Do a first visibility check
     var visible = elementIsInView(entry.el, entry.margin);
     if (entry.visible !== visible) {
       entry.visible = visible;
       APP.emit(entry.el, 'scrollVisibilityChanged', visible);
     }

     // Add or update entry
     scrollVisibilityElements[elID] = entry;

     // For IntersectionObserver: make new observer
     if (hasIntersectionObserver) {
       var observer = new IntersectionObserver(
           intersectionCallback,
           {rootMargin: '' + entry.margin + 'px'});
       observer.observe(entry.el);
       scrollVisibilityElements[elID].observer = observer;
     }

     // It's possible that the add was just an update of the options (margin change)
     if (!alreadyAdded)
       ++ nbScrollVisibilityElements;
   };

   this.remove = function(elID) {
     if (nbScrollVisibilityElements === 0)
        return;

     if (elID in scrollVisibilityElements) {
       // For IntersectionObserver: disconnect observer
       if (hasIntersectionObserver &&
           scrollVisibilityElements[elID].observer) {
         scrollVisibilityElements[elID].observer.disconnect();
       }

       delete scrollVisibilityElements[elID];
       -- nbScrollVisibilityElements;
     }

     // If nbScrollVisibilityElements === 0,
     // we can disable the callbacks
     if (nbScrollVisibilityElements === 0)
       disableVisibilityChecker();
   };
 }
);
