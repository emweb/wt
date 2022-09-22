/*
 * Copyright (C) 2012 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER(1, JavaScriptConstructor, "FlexLayout", function(APP, id) {
  const WT = APP.WT;

  function init() {
    const el = WT.getElement(id);
    if (!el) {
      return;
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
    }
  }

  setTimeout(init, 0);

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
      }
    }, 0);
  };
});
