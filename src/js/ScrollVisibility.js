/*
 * Copyright (C) 2016 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER(3, JavaScriptConstructor, "ScrollVisibility", function(APP) {
  const WT = APP.WT;

  function isVisible(w) {
    if (
      w.style.visibility === "hidden" ||
      w.style.display === "none" ||
      w.classList.contains("out")
    ) {
      return false;
    } else {
      w = w.parentNode;
      if (w && !WT.hasTag(w, "BODY")) {
        return isVisible(w);
      } else {
        return true;
      }
    }
  }

  let nbScrollVisibilityElements = 0;
  const scrollVisibilityElements = {};

  /*
   // Only support IntersectionObserver on desktop for now, because Chrome on Android
   // seems to be very late to notify transitions from visible to invisible.
   var hasIntersectionObserver = !WT.isAndroid && 'IntersectionObserver' in window;
   */
  // IntersectionObserver is still in early stages, let's not rely on it
  const hasIntersectionObserver = false; // FIXME: reenable IntersectionObserver when it matures?

  function elementIsInView(el, margin) {
    if (!isVisible(el)) {
      return false;
    }

    const p = WT.widgetPageCoordinates(el);
    const cx = p.x - document.body.scrollLeft - document.documentElement.scrollLeft;
    const cy = p.y - document.body.scrollTop - document.documentElement.scrollTop;
    const winSize = WT.windowSize();

    // Check intersection between viewport rectangle and element
    const x1 = cx;
    const w1 = el.offsetWidth;
    const y1 = cy;
    const h1 = el.offsetHeight;
    const x2 = -margin;
    const w2 = winSize.x + 2 * margin;
    const y2 = -margin;
    const h2 = winSize.y + 2 * margin;
    return x1 + w1 >= x2 &&
      x2 + w2 >= x1 &&
      y1 + h1 >= y2 &&
      y2 + h2 >= y1;
  }

  function visibilityChecker() {
    if (hasIntersectionObserver) {
      // The element may become visible by a change in the visibility
      // or the display property.
      for (const p of scrollVisibilityElements) {
        const visible = isVisible(scrollVisibilityElements[p].el);
        if (
          scrollVisibilityElements[p].visibleIfNotHidden &&
          scrollVisibilityElements[p].visible !== visible
        ) {
          scrollVisibilityElements[p].visible = visible;
          APP.emit(scrollVisibilityElements[p].el, "scrollVisibilityChanged", visible);
        }
      }
    } else {
      for (const p of scrollVisibilityElements) {
        const visible = elementIsInView(scrollVisibilityElements[p].el, scrollVisibilityElements[p].margin);
        if (visible !== scrollVisibilityElements[p].visible) {
          scrollVisibilityElements[p].visible = visible;
          APP.emit(scrollVisibilityElements[p].el, "scrollVisibilityChanged", visible);
        }
      }
    }
  }

  // For IntersectionObserver
  function intersectionCallback(entries) {
    for (const entry of entries) {
      const el = entry.target;
      const elID = el.id;
      const notHidden = isVisible(el);
      if (
        entry.intersectionRatio > 0 ||
        // FIXME: if the size of the element is 0,
        // it will still be considered visible if
        // top and left are nonzero. However, this
        // will also be zero if the element is in
        // the top left of the page.
        entry.intersectionRect.top !== 0 ||
        entry.intersectionRect.left !== 0
      ) {
        scrollVisibilityElements[elID].visibleIfNotHidden = true;
        if (scrollVisibilityElements[elID].visible !== notHidden) {
          scrollVisibilityElements[elID].visible = notHidden;
          APP.emit(scrollVisibilityElements[elID].el, "scrollVisibilityChanged", notHidden);
        }
      } else {
        scrollVisibilityElements[elID].visibleIfNotHidden = false;
        if (scrollVisibilityElements[elID].visible) {
          scrollVisibilityElements[elID].visible = false;
          APP.emit(scrollVisibilityElements[elID].el, "scrollVisibilityChanged", false);
        }
      }
    }
  }

  const mutObserver = new MutationObserver(visibilityChecker);

  function enableVisibilityChecker() {
    // Visibility can change if:
    //  - the DOM changes
    //  - the window is resized
    //  - some element is scrolled
    mutObserver.observe(document, { childList: true, attributes: true, subtree: true, characterData: true });
    if (!hasIntersectionObserver) {
      window.addEventListener("resize", visibilityChecker, true);
      window.addEventListener("scroll", visibilityChecker, true);
    }
  }

  function disableVisibilityChecker() {
    mutObserver.disconnect();
    if (!hasIntersectionObserver) {
      window.removeEventListener("resize", visibilityChecker, { capture: true });
      window.removeEventListener("scroll", visibilityChecker, { capture: true });
    }
  }

  this.add =
    /**
     * @param {Object} entry
     * @param {HTMLElement} entry.el the HTML element that has to be observed
     * @param {number} entry.margin the scrollVisibilityMargin in pixels
     * @param {boolean} entry.visible whether the element was visible previously
     * @param {boolean} [entry.visibleIfNotHidden] indicates whether the element is inside the viewport,
     *                                             only defined if the IntersectionObserver is enabled
     * @param {IntersectionObserver} [entry.observer] the IntersectionObserver for the element,
     *                                                only defined if the IntersectionObserver is enabled
     */
    function(entry) {
      // If nbScrollVisibilityElements === 0,
      // the checker was disabled and needs to be enabled
      if (nbScrollVisibilityElements === 0) {
        enableVisibilityChecker();
      }

      const elID = entry.el.id;
      const alreadyAdded = elID in scrollVisibilityElements;

      // For IntersectionObserver: disconnect old observer
      if (
        hasIntersectionObserver && alreadyAdded &&
        scrollVisibilityElements[elID].observer
      ) {
        scrollVisibilityElements[elID].observer.disconnect();
      }

      // Do a first visibility check
      const visible = elementIsInView(entry.el, entry.margin);
      if (entry.visible !== visible) {
        entry.visible = visible;
        APP.emit(entry.el, "scrollVisibilityChanged", visible);
      }

      // Add or update entry
      scrollVisibilityElements[elID] = entry;

      // For IntersectionObserver: make new observer
      if (hasIntersectionObserver) {
        const observer = new IntersectionObserver(
          intersectionCallback,
          { rootMargin: "" + entry.margin + "px" }
        );
        observer.observe(entry.el);
        scrollVisibilityElements[elID].observer = observer;
      }

      // It's possible that the add was just an update of the options (margin change)
      if (!alreadyAdded) {
        ++nbScrollVisibilityElements;
      }
    };

  this.remove = function(elID) {
    if (nbScrollVisibilityElements === 0) {
      return;
    }

    if (elID in scrollVisibilityElements) {
      // For IntersectionObserver: disconnect observer
      if (
        hasIntersectionObserver &&
        scrollVisibilityElements[elID].observer
      ) {
        scrollVisibilityElements[elID].observer.disconnect();
      }

      delete scrollVisibilityElements[elID];
      --nbScrollVisibilityElements;
    }

    // If nbScrollVisibilityElements === 0,
    // we can disable the callbacks
    if (nbScrollVisibilityElements === 0) {
      disableVisibilityChecker();
    }
  };
});
