/*
 * Copyright (C) 2012 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER(1, JavaScriptConstructor, "FlexLayout", function(APP, id, topLayout) {
  const WT = APP.WT;
  if (!WT.LayoutUninitialized) {
    WT.LayoutUninitialized = 1;
  } else {
    WT.LayoutUninitialized += 1;
  }

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
      el.style.height = "fit-content";
    }

    if (
      el.style.maxWidth !== "none" &&
      el.style.maxWidth !== "" &&
      (el.style.width === "auto" ||
        el.style.width === "")
    ) {
      el.style.width = "fit-content";
    }

    for (const c of el.childNodes) {
      if (
        c.style.display === "none" ||
        c.classList.contains("out") ||
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
      }

      if (unjustified.classList.contains("Wt-fill-height")) {
        const internalWrap = unjustified.children[0];
        const unwrapped = internalWrap.children[0];

        c.style.flexBasis = unwrapped.style.width;
        unjustified.style.flexBasis = unwrapped.style.width;
        internalWrap.style.flexBasis = unwrapped.style.height;
        unwrapped.style.height = "auto";
        copySizeLimits(unwrapped, unjustified);

        const top = WT.getElement(topLayout);
        setTimeout(function() {
          if (top && top.style.width !== "fit-content") {
            unwrapped.style.width = "auto";
          }
        }, 0);
      }
    }

    WT.LayoutUninitialized -= 1;
  }

  setTimeout(init, 0);

  this.resizeItem = function(item, width, height) {
    setTimeout(adjustItem, 0, item, width, height);
  };

  function adjustItem(item, width, height) {
    if (!item) {
      return;
    }

    let p = item.parentElement;
    if (!p) {
      return;
    }

    if (p.classList.contains("Wt-fill-width")) {
      p.style.flexBasis = height;
      item.style.height = "auto";

      copySizeLimits(item, p);
    }

    p = p.parentElement;
    if (!p) {
      return;
    }

    if (p.classList.contains("Wt-fill-height")) {
      const justifyWrap = p.parentElement;
      if (justifyWrap && justifyWrap.classList.contains("Wt-justify-wrap")) {
        justifyWrap.style.flexBasis = width;
      }
      p.style.flexBasis = width;
      const internalWrap = p.children[0];
      internalWrap.style.flexBasis = height;
      item.style.height = "auto";

      copySizeLimits(item, p);

      const top = WT.getElement(topLayout);
      setTimeout(function() {
        if (top && top.style.width !== "fit-content") {
          item.style.width = "auto";
        }
      }, 0);
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
      for (const c of children) {
        if (
          c.style.display === "none" ||
          c.classList.contains("out") ||
          c.className === "resize-sensor"
        ) {
          continue;
        }

        const flg = c.getAttribute("flg");
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

        const unjustified = c.classList.contains("Wt-justify-wrap") ? c.childNodes[0] : c;
        if (unjustified.classList.contains("Wt-fill-height")) {
          const top = WT.getElement(topLayout);
          if (top && top.style.width !== "fit-content") {
            const unwrapped = unjustified.childNodes[0].childNodes[0];
            unwrapped.style.width = "auto";
          }
        }
      }
    }, 0);
  };
});
