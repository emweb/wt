/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER(
  1,
  JavaScriptConstructor,
  "WTreeView",
  function(APP, el, contentsContainer, headerContainer, rowHeaderCount, selectedClass) {
    el.wtObj = this;

    const contents = contentsContainer.firstChild;
    const headers = headerContainer.firstChild;

    const self = this;
    const WT = APP.WT;

    let sizeSet = false;

    let itemDropsEnabled = false, betweenRowsDropsEnabled = false;

    const EnsureVisible = 0;
    const PositionAtTop = 1;
    const PositionAtBottom = 2;
    const PositionAtCenter = 3;

    function getItem(event) {
      let columnId = -1, nodeId = null, selected = false, drop = false, ele = null;

      let t = WT.target(event);

      while (t && t !== el) {
        if (WT.hasTag(t, "LI")) {
          if (columnId === -1) {
            columnId = 0;
          }
          nodeId = t.id;

          break;
        } else if (t.className && t.className.indexOf("Wt-tv-c") === 0) {
          if (t.className.indexOf("Wt-tv-c") === 0) {
            columnId = t.className.split(" ")[0].substring(7) * 1;
          } else if (columnId === -1) {
            columnId = 0;
          }
          if (t.getAttribute("drop") === "true") {
            drop = true;
          }
          ele = t;
        }

        if (t.classList.contains(selectedClass)) {
          selected = true;
        }

        t = t.parentNode;
      }

      return { columnId: columnId, nodeId: nodeId, selected: selected, drop: drop, el: ele };
    }

    this.click = function(obj, event) {
      const item = getItem(event);
      if (item.columnId !== -1) {
        APP.emit(
          el,
          { name: "itemEvent", eventObject: obj, event: event },
          item.nodeId + ":" + item.columnId,
          "clicked",
          "",
          ""
        );
      }
    };

    this.dblClick = function(obj, event) {
      const item = getItem(event);
      if (item.columnId !== -1) {
        APP.emit(
          el,
          { name: "itemEvent", eventObject: obj, event: event },
          item.nodeId + ":" + item.columnId,
          "dblclicked",
          "",
          ""
        );
      }
    };

    this.mouseDown = function(obj, event) {
      WT.capture(null);
      const item = getItem(event);
      if (item.columnId !== -1) {
        APP.emit(
          el,
          { name: "itemEvent", eventObject: obj, event: event },
          item.nodeId + ":" + item.columnId,
          "mousedown",
          "",
          ""
        );
        if (el.getAttribute("drag") === "true" && item.selected) {
          APP._p_.dragStart(el, event);
        }
      }
    };

    this.mouseUp = function(obj, event) {
      const item = getItem(event);
      if (item.columnId !== -1) {
        APP.emit(
          el,
          { name: "itemEvent", eventObject: obj, event: event },
          item.nodeId + ":" + item.columnId,
          "mouseup",
          "",
          ""
        );
      }
    };

    /** @type {?number} */
    let touchStartTimer = null;

    function emitTouchEvent(obj, event, evtType) {
      const item = getItem(event);
      if (item.columnId !== -1) {
        APP.emit(
          el,
          { name: "itemTouchEvent", eventObject: obj, event: event },
          item.nodeId + ":" + item.columnId,
          evtType
        );
      }
    }

    this.touchStart = function(obj, event) {
      // FIXME: emit touch start event if connected to slot
      // emitTouchEvent(obj, event, 'touchstart');
      if (event.touches.length > 1) {
        clearTimeout(touchStartTimer);
        touchStartTimer = setTimeout(function() {
          emitTouchEvent(obj, event, "touchselect");
        }, 1000);
      } else {
        clearTimeout(touchStartTimer);
        touchStartTimer = setTimeout(function() {
          emitTouchEvent(obj, event, "touchselect");
        }, 50);
      }
    };

    this.touchMove = function(obj, event) {
      if (event.touches.length === 1 && touchStartTimer) {
        clearTimeout(touchStartTimer);
        touchStartTimer = null;
      }
    };

    this.touchEnd = function(_obj, _event) {
      // FIXME: emit touch end event if connected to slot
      // emitTouchEvent(obj, event, 'touchend');
      if (touchStartTimer) {
        clearTimeout(touchStartTimer);
        touchStartTimer = null;
      }
    };

    this.rootClick = function(obj, event) {
      APP.emit(el, { name: "itemEvent", eventObject: obj, event: event }, "", "clicked", "", "");
    };

    this.rootDblClick = function(obj, event) {
      APP.emit(el, { name: "itemEvent", eventObject: obj, event: event }, "", "dblclicked", "", "");
    };

    this.rootMouseDown = function(obj, event) {
      APP.emit(el, { name: "itemEvent", eventObject: obj, event: event }, "", "mousedown", "", "");
    };

    this.rootMouseUp = function(obj, event) {
      APP.emit(el, { name: "itemEvent", eventObject: obj, event: event }, "", "mouseup", "", "");
    };

    this.resizeHandleMDown = function(obj, event) {
      const parent = obj.parentNode,
        c = parent.className.split(" ")[0];

      if (c) {
        const r = WT.getCssRule("#" + el.id + " ." + c),
          cw = WT.pxself(r, "width"),
          rtl = document.body.classList.contains("Wt-rtl");
        let minDelta = -cw,
          maxDelta = 10000;

        if (rtl) {
          [minDelta, maxDelta] = [-maxDelta, -minDelta];
        }

        new WT.SizeHandle(
          WT,
          "h",
          obj.offsetWidth,
          el.offsetHeight,
          minDelta,
          maxDelta,
          "Wt-hsh2",
          function(delta) {
            const newWidth = cw + (rtl ? -delta : delta),
              columnId = c.substring(7) * 1;
            r.style.width = newWidth + "px";
            self.adjustColumns();
            APP.emit(el, "columnResized", columnId, parseInt(newWidth));
          },
          obj,
          el,
          event,
          -2,
          -1
        );
      }
    };

    /*
    * this adjusts invariants that take into account column resizes
    *
    * c0w is set as soon as possible.
    *
    *  if (!column1 fixed):
    *    1) width('Wt-headerdiv') = sum(column widths)
    *    2) width('float: right') = sum(column(-1) widths)
    *    3) width(table parent) = width('Wt-headerdiv')
    *  else
    *    4) width('Wt-rowc') = sum(column(-1) widths)
    */
    let adjustScheduled = false;

    function doAdjustColumns() {
      if (!adjustScheduled || !sizeSet) {
        return;
      }

      if (WT.isHidden(el)) {
        return;
      }

      adjustScheduled = false;

      const wrapRoot = contents.firstChild; // table parent
      let hc = headers.firstChild, // Wt-tv-row
        allw_1 = 0,
        allw = 0;
      const c0id = headers.lastChild.className.split(" ")[0],
        c0r = WT.getCssRule("#" + el.id + " ." + c0id);

      if (rowHeaderCount) {
        hc = hc.firstChild; // Wt-tv-rowc
      }

      for (const childNode of hc.childNodes) {
        if (childNode.className) { // IE may have only a text node
          const cl = childNode.className.split(" ")[0],
            r = WT.getCssRule("#" + el.id + " ." + cl);

          if (r.style.display === "none") {
            continue;
          }

          // 7 = 2 * 3px (padding) + 1px border
          allw_1 += WT.pxself(r, "width") + 7;
        }
      }

      if (!rowHeaderCount) {
        if (!c0r.style.width) {
          // first resize and c0 width not set
          // 9 = 2px (widget border) - 7px column padding - 22px (scrollbar room)
          const c0rw = el.scrollWidth - hc.offsetWidth - 9 - 15;
          if (c0rw > 0) {
            c0r.style.width = c0rw + "px";
          }
        } else {
          el.querySelectorAll(`.Wt-headerdiv .${c0id}`).forEach(function(descendant) {
            descendant.style.width = c0r.style.width;
          });
        }
      }

      if (c0r.style["width"] === "auto") {
        return;
      }

      allw = allw_1 + WT.pxself(c0r, "width") + 7;

      if (!rowHeaderCount) {
        headers.style.width = wrapRoot.style.width = allw + "px";
        hc.style.width = allw_1 + "px";
      } else {
        const r = WT.getCssRule("#" + el.id + " .Wt-tv-rowc");
        r.style.width = allw_1 + "px";

        // if the WTreeView is rendered using StdGridLayoutImpl2,
        // the call to wtResize() is not necessary, because APP.layouts2.adjust()
        // already does that. It doesn't hurt, though.
        self.wtResize();
        if (APP.layouts2) {
          APP.layouts2.adjust();
        }
        el.changed = true;
      }
    }

    this.adjustColumns = function() {
      if (adjustScheduled) {
        return;
      }

      adjustScheduled = true;

      setTimeout(doAdjustColumns, 0);
    };

    this.setItemDropsEnabled = function(itemDrop) {
      itemDropsEnabled = itemDrop;
    };
    this.setRowDropsEnabled = function(betweenRowsDrop) {
      betweenRowsDropsEnabled = betweenRowsDrop;
    };

    let dropEl = null;
    let dropRow = null;

    function showRowDropSite(node, side) {
      // keep dropsite visual consistent by always displaying it at the bottom
      if (side === "top") {
        const prevSibling = node.previousSibling;
        if (prevSibling) {
          node = prevSibling;
          side = "bottom";
        }
      }

      const visual = document.createElement("div");
      visual.className = "Wt-drop-site-" + side;
      node.style.position = "relative";
      node.appendChild(visual);
      node.dropVisual = visual;

      dropRow = node;
    }
    function hideRowDropSite(node) {
      node.style.position = "";
      node.dropVisual.remove();
      delete node.dropVisual;
    }

    el.handleDragDrop = function(action, object, event, sourceId, mimeType) {
      if (dropEl) {
        dropEl.className = dropEl.classNameOrig;
        dropEl = null;
      }
      if (dropRow) {
        hideRowDropSite(dropRow);
        dropRow = null;
      }

      if (action === "end") {
        return;
      }

      const item = getItem(event);

      if (!item.selected && item.drop && itemDropsEnabled && item.columnId !== -1) {
        if (action === "drop") {
          APP.emit(
            el,
            { name: "itemEvent", eventObject: object, event: event },
            item.nodeId + ":" + item.columnId,
            "drop",
            sourceId,
            mimeType
          );
        } else {
          object.className = "Wt-valid-drop";
          dropEl = item.el;
          dropEl.classNameOrig = dropEl.className;
          dropEl.className = dropEl.className + " Wt-drop-site";
        }
      } else if (!item.selected && betweenRowsDropsEnabled) {
        let node;
        let side = "bottom";
        if (item.nodeId) {
          node = WT.$(item.nodeId);
          const coords = WT.widgetCoordinates(node, event);
          side = (coords.y - node.clientHeight / 2) <= 0 ? "top" : "bottom";
        } else {
          const root = contentsContainer.getElementsByClassName("Wt-tv-root");
          if (root.length === 1) {
            node = root[0].lastChild;
          } else {
            node = headerContainer;
          }
        }

        if (action === "drop") {
          APP.emit(
            el,
            { name: "rowDropEvent", eventObject: object, event: event },
            item.nodeId + ":" + item.columnId,
            sourceId,
            mimeType,
            side
          );
        } else {
          object.className = "Wt-valid-drop";
          showRowDropSite(node, side);
        }
      } else {
        object.className = "";
      }
    };

    /*
   * This adjusts invariants that depend on the size of the whole
   * treeview:
   *
   *  - changes to the total width (tw inc. affected by scrollbar)
   *  - when column1 is fixed:
   *    * .row width
   *    * table parent width
   */

    this.wtResize = function() {
      sizeSet = true;

      doAdjustColumns();

      let c0id;
      let c0r;
      let c0w = null;

      let tw = WT.pxself(el, "width");

      if (tw === 0) {
        tw = el.clientWidth;
      } else if (WT.boxSizing(el)) {
        tw -= WT.px(el, "borderLeftWidth");
        tw -= WT.px(el, "borderRightWidth");
      }

      let scrollwidth = contentsContainer.offsetWidth -
        contentsContainer.clientWidth;

      // IE cannot accurately estimate scrollwidth from time to time ?
      if (scrollwidth > 50) {
        scrollwidth = 0;
      }

      if (contentsContainer.clientWidth > 0) {
        tw -= scrollwidth;
      }

      if (el.classList.contains("column1")) {
        c0id = el.querySelector(".Wt-headerdiv")
          .lastChild.className.split(" ")[0];
        c0r = WT.getCssRule("#" + el.id + " ." + c0id);
        c0w = WT.pxself(c0r, "width");
      }

      // XXX: IE's incremental rendering foobars completely
      if (
        tw > 100 &&
        (tw !== contentsContainer.tw ||
          c0w !== contentsContainer.c0w ||
          el.changed)
      ) {
        const adjustColumns = !el.changed;

        contentsContainer.tw = tw;
        contentsContainer.c0w = c0w;

        c0id = el.querySelector(".Wt-headerdiv")
          .lastChild.className.split(" ")[0];
        c0r = WT.getCssRule("#" + el.id + " ." + c0id);

        const table = contents.firstChild,
          r = WT.getCssRule("#" + el.id + " .cwidth"),
          contentstoo = (r.style.width === (table.offsetWidth + 1) + "px"),
          hc = headers.firstChild;

        r.style.width = tw + "px";

        contentsContainer.style.width = (tw + scrollwidth) + "px";

        const rtl = document.body.classList.contains("Wt-rtl");
        if (!rtl) {
          headerContainer.style.marginRight = scrollwidth + "px";
          document.querySelectorAll(`#${el.id} .Wt-scroll`).forEach(function(descendant) {
            descendant.style.marginRight = scrollwidth + "px";
          });
        }

        if (c0w !== null) {
          const w = tw - c0w - 7;

          if (w > 0) {
            const w2 = Math.min(w, WT.pxself(WT.getCssRule("#" + el.id + " .Wt-tv-rowc"), "width"));
            tw -= w - w2;

            headers.style.width = tw + "px";
            table.style.width = tw + "px";

            /* This is really slow in FF, slower than the jquery equivalent */
            WT.getCssRule("#" + el.id + " .Wt-tv-row").style.width = w2 + "px";
          }
        } else {
          if (contentstoo) {
            headers.style.width = r.style.width;
            table.style.width = r.style.width;
          } else {
            headers.style.width = table.offsetWidth + "px";
          }
        }

        if (!rowHeaderCount && (table.offsetWidth - hc.offsetWidth >= 7)) {
          c0r.style.width = (table.offsetWidth - hc.offsetWidth - 7) + "px";
        }

        el.changed = false;

        if (adjustColumns) {
          self.adjustColumns();
        }
      }
    };

    this.scrollTo = function(_x, y, rowHeight, hint) {
      if (y !== -1) {
        y *= rowHeight;
        const top = contentsContainer.scrollTop,
          height = contentsContainer.clientHeight;
        if (hint === EnsureVisible) {
          if (top + height < y) {
            hint = PositionAtTop;
          } else if (y < top) {
            hint = PositionAtBottom;
          }
        }

        switch (hint) {
          case PositionAtTop:
            contentsContainer.scrollTop = y;
            break;
          case PositionAtBottom:
            contentsContainer.scrollTop = y - (height - rowHeight);
            break;
          case PositionAtCenter:
            contentsContainer.scrollTop = y - (height - rowHeight) / 2;
            break;
        }

        window.fakeEvent = { object: contentsContainer };
        contentsContainer.onscroll(window.fakeEvent);
        window.fakeEvent = null;
      }
    };

    /** @type {number} */
    let rowHeight = 0;
    this.setRowHeight = function(height) {
      rowHeight = height;
    };

    // bug #1352: when the browser is zoomed the alternating rowcolors get
    //   out of sync when scrolling down. fix = offset the alternating colors
    //   image to repeat starting from the current scrolled position.
    const offsetRowColorImg = function() {
      if (rowHeight === 0) {
        return;
      }

      const scrollPos = contentsContainer.scrollTop;
      const rootNode = contents.children[0].children[0];
      rootNode.style.backgroundPosition = "0px " + Math.floor(scrollPos / (2 * rowHeight)) * (2 * rowHeight) + "px";
    };

    if (contentsContainer.addEventListener) {
      contentsContainer.addEventListener("scroll", offsetRowColorImg);
    } else if (contentsContainer.attachEvent) {
      contentsContainer.attachEvent("onscroll", offsetRowColorImg);
    }

    self.adjustColumns();
  }
);
