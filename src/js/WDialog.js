/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER(
  1,
  JavaScriptConstructor,
  "WDialog",
  function(APP, el, titlebar, movable, centerX, centerY, movedSignal, resizedSignal, zIndexChangedSignal) {
    el.wtObj = this;

    const self = this;
    const layoutContainer = el.querySelector(".dialog-layout");
    const WT = APP.WT;
    let dsx, dsy;
    let x = -1, y = -1, w = -1, h = -1;
    let resizeBusy = false;
    // Percentage size before it was recomputed by layout manager. (if it was
    // set in %)
    let percentageWidth = -1, percentageHeight = -1;
    let calculatedPercentageWidth = -1, calculatedPercentageHeight = -1;

    function newPos() {
      if (movedSignal) {
        const newx = WT.pxself(el, "left");
        const newy = WT.pxself(el, "top");
        if (newx !== x || newy !== y) {
          x = newx;
          y = newy;
          APP.emit(el, movedSignal, x, y);
        }
      }
    }

    function newSize(neww, newh) {
      if (!resizeBusy) {
        if (neww !== w || newh !== h) {
          w = neww;
          h = newh;
          if (resizedSignal) {
            APP.emit(el, resizedSignal, w, h);
          }
        }
      }
    }

    function handleMove(event) {
      const e = event || window.event;
      const nowxy = WT.pageCoordinates(e);
      const wxy = WT.windowCoordinates(e);
      const wsize = WT.windowSize();

      if (wxy.x > 0 && wxy.x < wsize.x && wxy.y > 0 && wxy.y < wsize.y) {
        centerX = centerY = false;

        if (el.style.right === "auto" || el.style.right === "") {
          el.style.left = (WT.px(el, "left") + nowxy.x - dsx) + "px";
          el.style.right = "";
        } else {
          el.style.right = (WT.px(el, "right") + dsx - nowxy.x) + "px";
          el.style.left = "auto";
        }

        if (el.style.bottom === "auto" || el.style.bottom === "") {
          el.style.top = (WT.px(el, "top") + nowxy.y - dsy) + "px";
          el.style.bottom = "";
        } else {
          el.style.bottom = (WT.px(el, "bottom") + dsy - nowxy.y) + "px";
          el.style.top = "auto";
        }

        dsx = nowxy.x;
        dsy = nowxy.y;
      }
    }

    if (titlebar && movable) {
      titlebar.onmousedown = function(event) {
        const e = event || window.event;
        WT.capture(titlebar);
        const pc = WT.pageCoordinates(e);
        dsx = pc.x;
        dsy = pc.y;

        titlebar.onmousemove = handleMove;
      };

      titlebar.onmouseup = function(_event) {
        titlebar.onmousemove = null;

        newPos();

        WT.capture(null);
      };
    }

    this.centerDialog = function() {
      const pctMaxWidth = WT.parsePct(WT.css(el, "max-width"), 0);
      const pctMaxHeight = WT.parsePct(WT.css(el, "max-height"), 0);

      if (pctMaxWidth !== 0) {
        const ws = WT.windowSize();

        const layout = layoutContainer.firstChild.wtLayout;
        if (layout && layout.setMaxSize) {
          layout.setMaxSize(ws.x * pctMaxWidth / 100, ws.y * pctMaxHeight / 100);
        }
      }

      if (el.parentNode === null) {
        el = titlebar = null;
        return;
      }

      // FIXME: figure out the visibility story. The stdlayoutimpl sets
      //        itself to visible, seemingly by accident? It seems like
      //        a hack that centerDialog() causes the dialog to be visible?
      if ((el.style.display !== "none") /* && (el.style.visibility != 'hidden')*/) {
        const ws = WT.windowSize();
        const w = el.offsetWidth, h = el.offsetHeight;
        if (percentageWidth !== -1) {
          centerX = true;
        }

        if (percentageHeight !== -1) {
          centerY = true;
        }

        if (centerX) {
          el.style.left = Math.round(
            (ws.x - w) / 2 +
              (WT.isIE6 ? document.documentElement.scrollLeft : 0)
          ) + "px";
          el.style.marginLeft = "0px";
        }

        if (centerY) {
          el.style.top = Math.round(
            (ws.y - h) / 2 +
              (WT.isIE6 ? document.documentElement.scrollTop : 0)
          ) + "px";
          el.style.marginTop = "0px";
        }

        if (el.style.position !== "") {
          el.style.visibility = "visible";
        }

        newPos();
      }
    };

    /*
    * The dialog layout manager resizes the dialog
    */
    function layoutResize(ignored, w, h, setSize) {
      if (el.style.position === "") {
        el.style.position = WT.isIE6 ? "absolute" : "fixed";
      }

      el.style.visibility = "visible";

      percentageHeight = WT.parsePct(el.style.height, percentageHeight);
      percentageWidth = WT.parsePct(el.style.width, percentageWidth);

      if (setSize) {
        el.style.height = Math.max(0, h) + "px";
        el.style.width = Math.max(0, w) + "px";
      }

      newSize(w, h);

      self.centerDialog();

      const precentWidthChanged = percentageWidth !== -1;

      const precentHeightChanged = percentageHeight !== -1;

      if (precentWidthChanged && precentHeightChanged) {
        calculatedPercentageWidth = percentageWidthInPx();
        calculatedPercentageHeight = percentageHeightInPx();
        self.onresize(calculatedPercentageWidth, calculatedPercentageHeight, true);
      } else if (precentWidthChanged) {
        calculatedPercentageWidth = percentageWidthInPx();
        self.onresize(calculatedPercentageWidth, h, true);
      } else if (precentHeightChanged) {
        calculatedPercentageHeight = percentageHeightInPx();
        self.onresize(w, calculatedPercentageHeight, true);
      }
    }

    function percentageWidthInPx() {
      const ws = WT.windowSize();
      return (ws.x * percentageWidth / 100);
    }

    function percentageHeightInPx() {
      const ws = WT.windowSize();
      return (ws.y * percentageHeight / 100);
    }

    /*
    * C++ dialog.resize() was called
    */
    function wtResize(ignored, w, h, setSize) {
      if (setSize) {
        if (w > 0) {
          const leftWidth = WT.px(layoutContainer, "border-left-width");
          const rightWidth = WT.px(layoutContainer, "border-right-width");
          layoutContainer.style.width = `${w + rightWidth + leftWidth}px`;
        }
        if (h > 0) {
          const topWidth = WT.px(layoutContainer, "border-top-width");
          const bottomWidth = WT.px(layoutContainer, "border-bottom-width");
          layoutContainer.style.height = `${h + topWidth + bottomWidth}px`;
        }
      }

      self.centerDialog();

      if (el.wtResize) {
        el.wtResize(el, w, h, true);
      }
    }

    function wtPosition() {
      // if the WTreeView is rendered using StdGridLayoutImpl2,
      // the call to wtResize() is not necessary, because APP.layouts2.adjust()
      // already does that. It doesn't hurt, though.
      self.centerDialog();
      if (APP.layouts2) {
        // This is for StdGridLayoutImpl2,
        // will call self.centerDialog() as
        // part of its implementation
        APP.layouts2.adjust();
      }
    }

    this.bringToFront = function() {
      const maxz = WT.maxZIndex();
      if (maxz > el.style["zIndex"]) {
        const newZIndex = maxz + 1;
        el.style["zIndex"] = newZIndex;
        APP.emit(el, zIndexChangedSignal, newZIndex);
      }
    };

    /*
    * The user resizes the dialog using the resize handle
    */
    this.onresize = function(w, h, done) {
      centerX = centerY = false;

      resizeBusy = !done;
      wtResize(el, w, h, true);

      const layout = layoutContainer.firstChild.wtLayout;
      if (layout && layout.setMaxSize) {
        layout.setMaxSize(0, 0);
      }

      if (APP.layouts2) {
        APP.layouts2.scheduleAdjust();
      }

      if (done) {
        newSize(w, h);
      }
    };

    layoutContainer.wtResize = layoutResize;
    el.wtPosition = wtPosition;

    if (el.style.width !== "") {
      if (WT.parsePx(el.style.width) > 0) {
        layoutContainer.style.width = el.style.width;
      } else {
        layoutContainer.style.width = el.offsetWidth + "px";
      }
    }

    if (el.style.height !== "") {
      if (WT.parsePx(el.style.height) > 0) {
        layoutContainer.style.height = el.style.height;
      } else {
        layoutContainer.style.height = el.offsetHeight + "px";
      }
    }

    self.centerDialog();
  }
);
