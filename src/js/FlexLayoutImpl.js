/*
 * Copyright (C) 2012 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER(1, JavaScriptConstructor, "FlexLayout", function(APP, id, topLayout) {
  const WT = APP.WT;
  const CONTENT_SIZING_SHRINK_PRIORITY = "1000000";
  if (!WT.LayoutUninitialized) {
    WT.LayoutUninitialized = 1;
  } else {
    WT.LayoutUninitialized += 1;
  }

  /* Thickness of the visible resize handle, in pixels. */
  const HANDLE_SIZE = 5;

  /* Indices whose following border can be dragged. */
  let config = { resizable: [] };
  /* Live resize-handle elements, keyed by the boundary (item) index. */
  const handles = {};
  let resizeObserver = null;

  function copySizeLimits(from, to) {
    to.style.maxHeight = from.style.maxHeight;
    to.style.minHeight = from.style.minHeight;
    to.style.maxWidth = from.style.maxWidth;
    to.style.minWidth = from.style.minWidth;
  }

  function init() {
    const el = WT.getElement(id);
    if (!el) {
      WT.LayoutUninitialized -= 1;
      return;
    }

    if (
      el.style.maxHeight !== "none" &&
      el.style.maxHeight !== "" &&
      (el.style.height === "auto" ||
        el.style.height === "")
    ) {
      el.style.height = "max-content";
    }

    if (
      el.style.maxWidth !== "none" &&
      el.style.maxWidth !== "" &&
      (el.style.width === "auto" ||
        el.style.width === "")
    ) {
      el.style.width = "max-content";
    }

    for (const c of el.childNodes) {
      if (
        c.style.display === "none" ||
        c.classList.contains("out") ||
        c.classList.contains("Wt-flex-rh") ||
        c.className === "resize-sensor"
      ) {
        continue;
      }

      const overflow = WT.css(c, "overflow");
      if (overflow === "visible" || overflow === "") {
        c.style.overflow = "hidden";
      }

      const unjustified = c.classList.contains("Wt-justify-wrap") ? c.childNodes[0] : c;

      if (unjustified.classList.contains("Wt-fill-width")) {
        const unwrapped = unjustified.children[0];
        unjustified.style.flexBasis = unwrapped.style.height;

        unwrapped.style.height = "auto";
        copySizeLimits(unwrapped, unjustified);
        unwrapped.style.maxWidth = "100%";
        unwrapped.style.maxHeight = "100%";
      }

      if (unjustified.classList.contains("Wt-fill-height")) {
        const internalWrap = unjustified.children[0];
        const unwrapped = internalWrap.children[0];

        c.style.flexBasis = unwrapped.style.width;
        unjustified.style.flexBasis = unwrapped.style.width;
        internalWrap.style.flexBasis = unwrapped.style.height;
        unwrapped.style.height = "auto";
        copySizeLimits(unwrapped, unjustified);
        unwrapped.style.maxWidth = "100%";
        unwrapped.style.maxHeight = "100%";

        const top = WT.getElement(topLayout);
        setTimeout(function() {
          if (top && top.style.width !== "max-content") {
            unwrapped.style.width = "auto";
          }
        }, 0);
      }
    }

    WT.LayoutUninitialized -= 1;
  }

  setTimeout(init, 0);

  this.resizeItem = function(item, width, height, maxWidth, maxHeight, hasContentSizingKeyword) {
    setTimeout(adjustItem, 0, item, width, height, maxWidth, maxHeight, hasContentSizingKeyword);
  };

  function markContentSizingKeyword(item, hasContentSizingKeyword) {
    if (item.getAttribute("flg") === "0") {
      if (hasContentSizingKeyword) {
        item.setAttribute("csk", "1");
      } else {
        item.removeAttribute("csk");
      }
    }
  }

  function adjustItem(item, width, height, maxWidth, maxHeight, hasContentSizingKeyword) {
    if (!item) {
      return;
    }

    let p = item.parentElement;
    if (!p) {
      return;
    }

    const layoutElement = WT.getElement(id);
    if (layoutElement) {
      let flexItem = item;
      while (flexItem && flexItem.parentElement !== layoutElement) {
        flexItem = flexItem.parentElement;
      }
      if (flexItem) {
        markContentSizingKeyword(flexItem, hasContentSizingKeyword);
      }
    }

    let delagateMaxSize = false;

    if (p.classList.contains("Wt-fill-width")) {
      p.style.flexBasis = height;
      item.style.height = "auto";
      item.style.maxWidth = maxWidth;
      item.style.maxHeight = maxHeight;
      delagateMaxSize = true;

      copySizeLimits(item, p);
    }

    p = p.parentElement;

    if (p && p.classList.contains("Wt-fill-height")) {
      const justifyWrap = p.parentElement;
      if (justifyWrap && justifyWrap.classList.contains("Wt-justify-wrap")) {
        justifyWrap.style.flexBasis = width;
      }
      p.style.flexBasis = width;
      const internalWrap = p.children[0];
      internalWrap.style.flexBasis = height;
      item.style.height = "auto";
      item.style.maxWidth = maxWidth;
      item.style.maxHeight = maxHeight;
      delagateMaxSize = true;

      copySizeLimits(item, p);

      const top = WT.getElement(topLayout);
      setTimeout(function() {
        if (top && top.style.width !== "max-content") {
          item.style.width = "auto";
        }
      }, 0);
    }

    if (delagateMaxSize) {
      item.style.maxWidth = "100%";
      item.style.maxHeight = "100%";
    }
  }

  this.adjust = function() {
    setTimeout(function() {
      const el = WT.getElement(id);
      if (!el) {
        return;
      }

      const children = el.childNodes;
      let totalStretch = 0;
      let hasContentSizingItem = false;
      const vertical = el.style.flexFlow.startsWith("column");
      const fitContentContainer = vertical ?
        el.style.height === "max-content" :
        el.style.width === "max-content";
      for (const c of children) {
        if (
          c.style.display === "none" ||
          c.classList.contains("out") ||
          c.classList.contains("Wt-flex-rh") ||
          c.className === "resize-sensor"
        ) {
          continue;
        }

        const flg = c.getAttribute("flg");
        if (c.getAttribute("csk") === "1") {
          hasContentSizingItem = true;
        }
        if (flg === "0") {
          continue;
        }

        const flexGrow = WT.css(c, "flex-grow");
        totalStretch += parseFloat(flexGrow);
      }

      for (const c of children) {
        if (
          c.style.display === "none" ||
          c.classList.contains("out") ||
          c.classList.contains("Wt-flex-rh") ||
          c.className === "resize-sensor"
        ) {
          continue;
        }

        /* Re-trigger resize-sensor */
        if (c.resizeSensor) {
          c.resizeSensor.trigger();
        }

        let stretch;

        if (totalStretch === 0) {
          stretch = 1;
        } else {
          const flg = c.getAttribute("flg");
          if (flg === "0") {
            stretch = 0;
          } else {
            const flexGrow = WT.css(c, "flex-grow");
            stretch = flexGrow;
          }
        }

        c.style.flexGrow = stretch;
        if (fitContentContainer) {
          c.style.flexShrink = "0";
        } else if (totalStretch === 0) {
          c.style.flexShrink = "1";
        } else if (stretch !== 0) {
          c.style.flexShrink = hasContentSizingItem ? CONTENT_SIZING_SHRINK_PRIORITY : "1";
        } else if (c.getAttribute("csk") === "1") {
          c.style.flexShrink = "1";
        } else {
          c.style.flexShrink = "0";
        }

        const unjustified = c.classList.contains("Wt-justify-wrap") ? c.childNodes[0] : c;
        if (unjustified.classList.contains("Wt-fill-height")) {
          const top = WT.getElement(topLayout);
          if (top && top.style.width !== "max-content") {
            const unwrapped = unjustified.childNodes[0].childNodes[0];
            unwrapped.style.width = "auto";
          }
        }
      }

      positionHandles();
    }, 0);
  };

  /*
   * Resize handles.
   *
   * A resize handle is an absolutely positioned element between two
   * adjacent items. It is appended after all item children so it never
   * disturbs server-side positional inserts. Dragging fixes one section at
   * a new size and lets Flex redistribute the remaining space, matching the
   * JavaScript layout implementation.
   */

  this.setConfig = function(c) {
    config = c;
    setTimeout(reconcileHandles, 0);
  };

  function isHorizontal(el) {
    const dir = WT.css(el, "flex-direction");
    return dir === "row" || dir === "row-reverse" || dir === "";
  }

  /* Direct item children in section order. Hidden items remain in the
   * list so their indices match the server-side configuration. */
  function itemChildren(el) {
    const result = [];
    for (const c of el.childNodes) {
      if (
        !c.classList ||
        c.classList.contains("out") ||
        c.classList.contains("Wt-flex-rh") ||
        c.className === "resize-sensor"
      ) {
        continue;
      }
      result.push(c);
    }
    return result;
  }

  function minSize(item, horizontal) {
    const v = WT.px(item, horizontal ? "min-width" : "min-height");
    return v > 0 ? v : 0;
  }

  function maxSize(item, horizontal) {
    const prop = horizontal ? "max-width" : "max-height";
    const raw = WT.css(item, prop);
    if (!raw || raw === "none") {
      return 0;
    }
    const v = WT.px(item, prop);
    return v > 0 ? v : 0;
  }

  function isHidden(item) {
    return item.style.display === "none";
  }

  function isStretchable(item) {
    if (typeof item.wtFlexStretch === "undefined") {
      item.wtFlexStretch = item.getAttribute("flg") !== "0";
    }
    return item.wtFlexStretch;
  }

  function itemSize(item, horizontal) {
    const rect = item.getBoundingClientRect();
    return horizontal ? rect.width : rect.height;
  }

  function setItemSize(item, px) {
    item.style.flexGrow = "0";
    item.style.flexShrink = "0";
    /*
     * The size comes from getBoundingClientRect(), so use border-box
     * sizing when pinning the section.
     */
    item.style.boxSizing = "border-box";
    item.style.flexBasis = px + "px";
    item.setAttribute("flg", "0");
  }

  function reconcileHandles() {
    const el = WT.getElement(id);
    if (!el) {
      return;
    }

    const resizable = new Set(config.resizable || []);
    const items = itemChildren(el);
    const horizontal = isHorizontal(el);
    const handleClass = "Wt-flex-rh " + (horizontal ? "Wt-vrh2" : "Wt-hrh2");

    /* Remove handles that are no longer valid. */
    for (const key of Object.keys(handles)) {
      const i = parseInt(key, 10);
      if (!resizable.has(i) || i + 1 >= items.length) {
        const h = handles[key];
        if (h && h.parentNode) {
          h.parentNode.removeChild(h);
        }
        delete handles[key];
      }
    }

    /* Create handles for borders that have a following item. */
    let anyHandle = false;
    for (let i = 0; i + 1 < items.length; ++i) {
      if (!resizable.has(i)) {
        continue;
      }
      anyHandle = true;
      let h = handles[i];
      if (!h) {
        h = document.createElement("div");
        h.di = i;
        h.style.position = "absolute";
        h.onmousedown = h.ontouchstart = function(event) {
          startResize(this.di, event || window.event);
        };
        el.appendChild(h);
        handles[i] = h;
      }
      h.className = handleClass;
    }

    if (anyHandle) {
      if (WT.css(el, "position") === "static") {
        el.style.position = "relative";
      }
      if (!resizeObserver && window.ResizeObserver) {
        resizeObserver = new ResizeObserver(function() {
          positionHandles();
        });
        resizeObserver.observe(el);
      }
    } else if (resizeObserver) {
      resizeObserver.disconnect();
      resizeObserver = null;
    }

    positionHandles();
  }

  function positionHandles() {
    const el = WT.getElement(id);
    if (!el) {
      return;
    }

    const items = itemChildren(el);
    const horizontal = isHorizontal(el);
    const elRect = el.getBoundingClientRect();
    const originLeft = elRect.left + el.clientLeft;
    const originTop = elRect.top + el.clientTop;
    const paddingLeft = WT.pxComputedStyle(el, "padding-left");
    const paddingRight = WT.pxComputedStyle(el, "padding-right");
    const paddingTop = WT.pxComputedStyle(el, "padding-top");
    const paddingBottom = WT.pxComputedStyle(el, "padding-bottom");

    for (const key of Object.keys(handles)) {
      const i = parseInt(key, 10);
      const h = handles[key];

      if (i + 1 >= items.length || isHidden(items[i]) || isHidden(items[i + 1])) {
        h.style.display = "none";
        continue;
      }
      h.style.display = "";

      const a = items[i].getBoundingClientRect();
      const b = items[i + 1].getBoundingClientRect();

      if (horizontal) {
        const nearEdge = a.left < b.left ? a.right : b.right;
        const farEdge = a.left < b.left ? b.left : a.left;
        const center = (nearEdge + farEdge) / 2;
        h.style.left = (center - originLeft - HANDLE_SIZE / 2) + "px";
        h.style.width = HANDLE_SIZE + "px";
        h.style.top = paddingTop + "px";
        h.style.height = (el.clientHeight - paddingTop - paddingBottom) + "px";
      } else {
        const nearEdge = a.top < b.top ? a.bottom : b.bottom;
        const farEdge = a.top < b.top ? b.top : a.top;
        const center = (nearEdge + farEdge) / 2;
        h.style.top = (center - originTop - HANDLE_SIZE / 2) + "px";
        h.style.height = HANDLE_SIZE + "px";
        h.style.left = paddingLeft + "px";
        h.style.width = (el.clientWidth - paddingLeft - paddingRight) + "px";
      }
    }
  }

  function maxSizeIncrease(items, resized, horizontal) {
    let stretchable = 0;
    let fixed = 0;
    let hasStretchable = false;

    for (const item of items) {
      if (item === resized || isHidden(item)) {
        continue;
      }

      const available = itemSize(item, horizontal) -
        minSize(item, horizontal);
      if (isStretchable(item)) {
        stretchable += available;
        hasStretchable = true;
      } else {
        fixed += available;
      }
    }

    return hasStretchable ? stretchable : fixed;
  }

  function startResize(i, event) {
    const el = WT.getElement(id);
    if (!el) {
      return;
    }

    const items = itemChildren(el);
    if (i + 1 >= items.length || isHidden(items[i]) || isHidden(items[i + 1])) {
      return;
    }

    const horizontal = isHorizontal(el);
    const h = handles[i];
    const before = items[i];
    const after = items[i + 1];
    const resized = isStretchable(before) && !isStretchable(after) ?
      after :
      before;
    const other = resized === before ? after : before;
    const resizedRect = resized.getBoundingClientRect();
    const otherRect = other.getBoundingClientRect();
    const visuallyFirst = horizontal ?
      resizedRect.left < otherRect.left :
      resizedRect.top < otherRect.top;
    const direction = visuallyFirst ? 1 : -1;
    const size = itemSize(resized, horizontal);
    const minimum = minSize(resized, horizontal);
    let maximum = size + maxSizeIncrease(items, resized, horizontal);
    const configuredMaximum = maxSize(resized, horizontal);
    if (configuredMaximum > 0) {
      maximum = Math.min(maximum, configuredMaximum);
    }

    const minDelta = direction > 0 ? minimum - size : size - maximum;
    const maxDelta = direction > 0 ? maximum - size : size - minimum;

    new WT.SizeHandle(
      WT,
      horizontal ? "h" : "v",
      HANDLE_SIZE,
      horizontal ? h.offsetHeight : h.offsetWidth,
      minDelta,
      maxDelta,
      /* Keep the transient bar out of the item loops. */
      (horizontal ? "Wt-hsh2" : "Wt-vsh2") + " Wt-flex-rh",
      function(delta) {
        let newSize = size + direction * delta;
        if (newSize < minimum) {
          newSize = minimum;
        }
        if (newSize > maximum) {
          newSize = maximum;
        }
        setItemSize(resized, newSize);
        setTimeout(positionHandles, 0);
      },
      h,
      el,
      event,
      0,
      0
    );
  }
});
