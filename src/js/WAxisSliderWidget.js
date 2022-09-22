/*
 * Copyright (C) 2015 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER(1, JavaScriptConstructor, "WAxisSliderWidget", function(APP, widget, target, config) {
  // draw area: inside of the margins of the widget
  // config: { chart:, rect:(function), transform:, drawArea:, series:, updateYAxis: }
  let framePending = false;
  const rqAnimFrameThrottled = function(cb) {
    if (framePending) {
      return;
    }
    framePending = true;
    requestAnimationFrame(function() {
      cb();
      framePending = false;
    });
  };

  const touchHandlers = {};

  widget.wtSObj = this;

  const self = this;
  const WT = APP.WT;

  target.canvas.style.msTouchAction = "none";

  function isTouchEvent(event) {
    return event.pointerType === 2 || event.pointerType === 3 ||
      event.pointerType === "pen" || event.pointerType === "touch";
  }

  let pointerActive = false;

  if (window.MSPointerEvent || window.PointerEvent) {
    widget.style.touchAction = "none";
    target.canvas.style.msTouchAction = "none";
    target.canvas.style.touchAction = "none";
  }

  if (window.MSPointerEvent || window.PointerEvent) {
    (function() {
      const pointers = [];

      function updatePointerActive() {
        if (pointers.length > 0 && !pointerActive) {
          pointerActive = true;
        } else if (pointers.length <= 0 && pointerActive) {
          pointerActive = false;
        }
      }

      function pointerDown(event) {
        if (!isTouchEvent(event)) {
          return;
        }
        event.preventDefault();
        pointers.push(event);

        updatePointerActive();
        touchHandlers.start(widget, { touches: pointers.slice(0) });
      }

      function pointerUp(event) {
        if (!pointerActive) {
          return;
        }
        if (!isTouchEvent(event)) {
          return;
        }
        event.preventDefault();
        for (let i = 0; i < pointers.length; ++i) {
          if (pointers[i].pointerId === event.pointerId) {
            pointers.splice(i, 1);
            break;
          }
        }

        updatePointerActive();
        touchHandlers.end(widget, { touches: pointers.slice(0), changedTouches: [] });
      }

      function pointerMove(event) {
        if (!isTouchEvent(event)) {
          return;
        }
        event.preventDefault();
        for (let i = 0; i < pointers.length; ++i) {
          if (pointers[i].pointerId === event.pointerId) {
            pointers[i] = event;
            break;
          }
        }

        updatePointerActive();
        touchHandlers.moved(widget, { touches: pointers.slice(0) });
      }

      const o = widget.wtEObj;
      if (o) {
        widget.removeEventListener("pointerdown", o.pointerDown);
        widget.removeEventListener("pointerup", o.pointerUp);
        widget.removeEventListener("pointerout", o.pointerUp);
        widget.removeEventListener("pointermove", o.pointerMove);
      }
      widget.wtEObj = {
        pointerDown: pointerDown,
        pointerUp: pointerUp,
        pointerMove: pointerMove,
      };
      widget.addEventListener("pointerdown", pointerDown);
      widget.addEventListener("pointerup", pointerUp);
      widget.addEventListener("pointerout", pointerUp);
      widget.addEventListener("pointermove", pointerMove);
    })();
  }

  const left = WT.gfxUtils.rect_left,
    right = WT.gfxUtils.rect_right,
    top = WT.gfxUtils.rect_top,
    bottom = WT.gfxUtils.rect_bottom;

  function configSeries() {
    return config.chart.config.series[config.series];
  }

  function seriesXAxis() {
    return configSeries().xAxis;
  }
  this.xAxis = seriesXAxis;

  function minXZoom() {
    return config.chart.config.minZoom.x[seriesXAxis()];
  }
  function maxXZoom() {
    return config.chart.config.maxZoom.x[seriesXAxis()];
  }

  let previousXY = null;

  // positions:
  const LEFT_OF_RECT = 1, ON_RECT = 2, RIGHT_OF_RECT = 3;
  let position = null;

  function scheduleRepaint() {
    rqAnimFrameThrottled(target.repaint);
  }

  this.changeRange = function(u, v) {
    if (u < 0) {
      u = 0;
    }
    if (v > 1) {
      v = 1;
    }
    const drawArea = config.drawArea;
    config.transform[0] = v - u;
    config.transform[4] = u * drawArea[2];
    scheduleRepaint();
  };

  function transformToUV() {
    const drawArea = config.drawArea;
    const u = config.transform[4] / drawArea[2];
    const v = config.transform[0] + u;
    return [u, v];
  }

  function repaint(setRange) {
    scheduleRepaint();
    if (setRange) {
      const transform = config.transform;
      const drawArea = config.drawArea;
      const u = transform[4] / drawArea[2];
      const v = transform[0] + u;
      config.chart.setXRange(config.series, u, v, config.updateYAxis);
    }
  }

  function isHorizontal() {
    return !config.chart.config.isHorizontal;
  }

  function onLeftBorder(p, rect, borderSize) {
    if (isHorizontal()) {
      return p.y >= top(rect) && p.y <= bottom(rect) && p.x > left(rect) - borderSize / 2 &&
        p.x < left(rect) + borderSize / 2;
    } else {
      return p.x >= left(rect) && p.x <= right(rect) && p.y > top(rect) - borderSize / 2 &&
        p.y < top(rect) + borderSize / 2;
    }
  }

  function onRightBorder(p, rect, borderSize) {
    if (isHorizontal()) {
      return p.y >= top(rect) && p.y <= bottom(rect) && p.x > right(rect) - borderSize / 2 &&
        p.x < right(rect) + borderSize / 2;
    } else {
      return p.x >= left(rect) && p.x <= right(rect) && p.y > bottom(rect) - borderSize / 2 &&
        p.y < bottom(rect) + borderSize / 2;
    }
  }

  function isInside(p, rect) {
    if (isHorizontal()) {
      return p.y >= top(rect) && p.y <= bottom(rect) && p.x > left(rect) && p.x < right(rect);
    } else {
      return p.x >= left(rect) && p.x <= right(rect) && p.y > top(rect) && p.y < bottom(rect);
    }
  }

  this.mouseDown = function(o, event) {
    if (pointerActive) {
      return;
    }
    previousXY = WT.widgetCoordinates(widget, event);
    const rect = config.rect();
    if (onLeftBorder(previousXY, rect, 10)) {
      position = LEFT_OF_RECT;
    } else if (onRightBorder(previousXY, rect, 10)) {
      position = RIGHT_OF_RECT;
    } else if (isInside(previousXY, rect)) {
      position = ON_RECT;
    } else {
      position = null;
      return;
    }
    WT.cancelEvent(event);
  };

  this.mouseUp = function(o, event) {
    if (pointerActive) {
      return;
    }
    previousXY = null;
    if (position === null) {
      return;
    }
    position = null;
    WT.cancelEvent(event);
  };

  function dragLeft(dx) {
    const transform = config.transform;
    const drawArea = config.drawArea;
    const u = transform[4] / drawArea[2];
    const v = transform[0] + u;
    const xBefore = u * drawArea[2];
    const xAfter = xBefore + dx;
    let uAfter = xAfter / drawArea[2];
    if (v <= uAfter) {
      return;
    }
    const newZoom = 1 / (v - uAfter);
    if (newZoom > maxXZoom()) {
      return;
    }
    if (newZoom < minXZoom()) {
      return;
    }
    if (uAfter < 0) {
      uAfter = 0;
    }
    if (uAfter > 1) {
      uAfter = 1;
    }
    self.changeRange(uAfter, v);
    repaint(true);
    const uv = transformToUV();
    if (Math.abs(uv[1] - v) > Math.abs(uv[0] - u)) {
      self.changeRange(u, v);
      repaint(true);
    }
  }

  function dragRight(dx) {
    const transform = config.transform;
    const drawArea = config.drawArea;
    const u = transform[4] / drawArea[2];
    const v = transform[0] + u;
    const xBefore = v * drawArea[2];
    const xAfter = xBefore + dx;
    let vAfter = xAfter / drawArea[2];
    if (vAfter <= u) {
      return;
    }
    const newZoom = 1 / (vAfter - u);
    if (newZoom > maxXZoom()) {
      return;
    }
    if (newZoom < minXZoom()) {
      return;
    }
    if (vAfter < 0) {
      vAfter = 0;
    }
    if (vAfter > 1) {
      vAfter = 1;
    }
    self.changeRange(u, vAfter);
    repaint(true);
    const uv = transformToUV();
    if (Math.abs(uv[0] - u) > Math.abs(uv[1] - v)) {
      self.changeRange(u, v);
      repaint(true);
    }
  }

  function move(dx) {
    const transform = config.transform;
    const drawArea = config.drawArea;
    const u = transform[4] / drawArea[2];
    const v = transform[0] + u;
    const leftBefore = u * drawArea[2];
    let leftAfter = leftBefore + dx;
    if (leftAfter < 0) {
      dx = -leftBefore;
      leftAfter = 0;
    }
    let uAfter = leftAfter / drawArea[2];
    const rightBefore = v * drawArea[2];
    let rightAfter = rightBefore + dx;
    if (rightAfter > drawArea[2]) {
      dx = drawArea[2] - rightBefore;
      leftAfter = leftBefore + dx;
      uAfter = leftAfter / drawArea[2];
      rightAfter = drawArea[2];
    }
    const vAfter = rightAfter / drawArea[2];
    self.changeRange(uAfter, vAfter);
    repaint(true);
  }

  this.mouseDrag = function(o, event) {
    if (pointerActive) {
      return;
    }
    if (!position) {
      return;
    }
    WT.cancelEvent(event);
    const pos = WT.widgetCoordinates(widget, event);
    if (previousXY === null) {
      previousXY = pos;
      return;
    }
    let dx;
    if (isHorizontal()) {
      dx = pos.x - previousXY.x;
    } else {
      dx = pos.y - previousXY.y;
    }
    switch (position) {
      case LEFT_OF_RECT:
        dragLeft(dx);
        break;
      case ON_RECT:
        move(dx);
        break;
      case RIGHT_OF_RECT:
        dragRight(dx);
        break;
    }
    previousXY = pos;
    repaint(true);
  };

  this.mouseMoved = function(o, event) {
    setTimeout(function() {
      if (pointerActive) {
        return;
      }
      if (position) {
        return;
      }
      const pos = WT.widgetCoordinates(widget, event);
      const rect = config.rect();
      if (onLeftBorder(pos, rect, 10) || onRightBorder(pos, rect, 10)) {
        if (isHorizontal()) {
          target.canvas.style.cursor = "col-resize";
        } else {
          target.canvas.style.cursor = "row-resize";
        }
      } else if (isInside(pos, rect)) {
        target.canvas.style.cursor = "move";
      } else {
        target.canvas.style.cursor = "auto";
      }
    }, 0);
  };

  let singleTouch = false;
  let doubleTouch = false;

  let touchDelta = null;

  touchHandlers.start = function(o, event) {
    singleTouch = event.touches.length === 1;
    doubleTouch = event.touches.length === 2;
    if (singleTouch) {
      previousXY = WT.widgetCoordinates(target.canvas, event.touches[0]);
      const rect = config.rect();
      if (onLeftBorder(previousXY, rect, 20)) {
        position = LEFT_OF_RECT;
      } else if (onRightBorder(previousXY, rect, 20)) {
        position = RIGHT_OF_RECT;
      } else if (isInside(previousXY, rect)) {
        position = ON_RECT;
      } else {
        position = null;
        return;
      }
      WT.capture(null);
      WT.capture(target.canvas);
      if (event.preventDefault) {
        event.preventDefault();
      }
    } else if (doubleTouch) {
      position = null;
      const touches = [
        WT.widgetCoordinates(target.canvas, event.touches[0]),
        WT.widgetCoordinates(target.canvas, event.touches[1]),
      ];
      const rect = config.rect();
      if (
        !isInside(touches[0], rect) ||
        !isInside(touches[1], rect)
      ) {
        return;
      }
      if (isHorizontal()) {
        touchDelta = Math.abs(touches[0].x - touches[1].x);
      } else {
        touchDelta = Math.abs(touches[0].y - touches[1].y);
      }
      WT.capture(null);
      WT.capture(target.canvas);
      if (event.preventDefault) {
        event.preventDefault();
      }
    }
  };

  touchHandlers.end = function(o, event) {
    const touches = Array.prototype.slice.call(event.touches);

    const wasSingleTouch = singleTouch;
    const wasDoubleTouch = doubleTouch;
    let noTouch = touches.length === 0;
    singleTouch = touches.length === 1;
    doubleTouch = touches.length === 2;

    if (!noTouch) {
      (function() {
        for (let i = 0; i < event.changedTouches.length; ++i) {
          (function() {
            const id = event.changedTouches[i].identifier;
            for (let j = 0; j < touches.length; ++j) {
              if (touches[j].identifier === id) {
                touches.splice(j, 1);
                return;
              }
            }
          })();
        }
      })();
    }

    noTouch = touches.length === 0;
    singleTouch = touches.length === 1;
    doubleTouch = touches.length === 2;

    if (noTouch && wasSingleTouch) {
      previousXY = null;
      if (position === null) {
        return;
      }
      position = null;
      WT.cancelEvent(event);
    }
    if (singleTouch && wasDoubleTouch) {
      doubleTouch = false;
      touchDelta = null;
      WT.cancelEvent(event);
      touchHandlers.start(widget, event);
    }
    if (noTouch && wasDoubleTouch) {
      doubleTouch = false;
      touchDelta = null;
      WT.cancelEvent(event);
    }
  };

  touchHandlers.moved = function(o, event) {
    if (position) {
      if (event.preventDefault) {
        event.preventDefault();
      }
      const pos = WT.widgetCoordinates(widget, event);
      if (previousXY === null) {
        previousXY = pos;
        return;
      }
      let dx;
      if (isHorizontal()) {
        dx = pos.x - previousXY.x;
      } else {
        dx = pos.y - previousXY.y;
      }
      switch (position) {
        case LEFT_OF_RECT:
          dragLeft(dx);
          break;
        case ON_RECT:
          move(dx);
          break;
        case RIGHT_OF_RECT:
          dragRight(dx);
          break;
      }
      previousXY = pos;
      repaint(true);
    } else if (doubleTouch) {
      if (event.preventDefault) {
        event.preventDefault();
      }
      const touches = [
        WT.widgetCoordinates(target.canvas, event.touches[0]),
        WT.widgetCoordinates(target.canvas, event.touches[1]),
      ];
      let newDelta;
      if (isHorizontal()) {
        newDelta = Math.abs(touches[0].x - touches[1].x);
      } else {
        newDelta = Math.abs(touches[0].y - touches[1].y);
      }
      const d = newDelta - touchDelta;
      dragLeft(-d / 2);
      dragRight(d / 2);
      touchDelta = newDelta;
      repaint(true);
    }
  };

  this.updateConfig = function(newConfig) {
    for (const [key, value] of Object.entries(newConfig)) {
      config[key] = value;
    }
    repaint(false);
  };

  self.updateConfig({});

  if (window.TouchEvent && !window.MSPointerEvent && !window.PointerEvent) {
    self.touchStarted = touchHandlers.start;
    self.touchEnded = touchHandlers.end;
    self.touchMoved = touchHandlers.moved;
  } else {
    const nop = function() {};
    self.touchStarted = nop;
    self.touchEnded = nop;
    self.touchMoved = nop;
  }
});
