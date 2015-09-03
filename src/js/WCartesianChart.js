/*
 * Copyright (C) 2015 Emweb bvba, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER
(1, JavaScriptConstructor, "WCartesianChart",
 // target: the WPaintedWidget JavaScript obj, with:
 //   repaint
 //   canvas
 // config: the initial configuration (can be overridden with updateConfig)
 //     isHorizontal (if orientation() == Horizontal)
 //	xTransform
 //	yTransform
 //	area (WRectF describing render area)
 //	modelArea (WRectF describing model area)
 //	maxZoom
 //	rubberBand
 //	zoom (bool)
 //	pan (bool)
 //	crosshair (bool)
 //	followCurve (int, -1 for disabled)
 //	series {modelColumn: series ref,...}
 //
 function(APP, widget, target, config) {
   var ANIMATION_INTERVAL = 17;
   var rqAnimFrame = (function(){
      return window.requestAnimationFrame       ||
	     window.webkitRequestAnimationFrame ||
	     window.mozRequestAnimationFrame    ||
             function(callback) {
		window.setTimeout(callback, ANIMATION_INTERVAL);
	     };
   })();

   if (window.MSPointerEvent || window.PointerEvent) {
      widget.style.touchAction = 'none';
      target.canvas.style.msTouchAction = 'none';
      target.canvas.style.touchAction = 'none';
   }

   var MOVE_TO = 0, LINE_TO = 1, CUBIC_C1 = 2, CUBIC_C2 = 3, CUBIC_END = 4,
       QUAD_C = 5, QUAD_END = 6, ARC_C = 7, ARC_R = 8, ARC_ANGLE_SWEEP = 9;
   var NO_LIMIT = 1, DAMPEN = 2; // bit flags
   var X_ONLY = 1, Y_ONLY = 2; // bit flags
   var X = 0, Y = 1;
   var LOOK_MODE = 0, CROSSHAIR_MODE = 1;
   var WHEEL_ZOOM_X = 0, WHEEL_ZOOM_Y = 1, WHEEL_ZOOM_XY = 2,
       WHEEL_ZOOM_MATCHING = 3, WHEEL_PAN_X = 4, WHEEL_PAN_Y = 5,
       WHEEL_PAN_MATCHING = 6;

   var touchHandlers = {};

   function showCrosshair() {
      return config.crosshair || config.followCurve !== -1;
   }

   function isTouchEvent(event) {
      return event.pointerType === 2 || event.pointerType === 3 ||
	     event.pointerType === 'pen' || event.pointerType === 'touch';
   }

   var pointerActive = false;

   if (window.MSPointerEvent || window.PointerEvent) {
      (function(){
	 pointers = []

	 function updatePointerActive() {
	    if (pointers.length > 0 && !pointerActive) {
	       pointerActive = true;
	    } else if (pointers.length <= 0 && pointerActive) {
	       pointerActive = false;
	    }
	 }

	 function pointerDown(event) {
	    if (!isTouchEvent(event)) return;
	    event.preventDefault();
	    pointers.push(event);

	    updatePointerActive();
	    touchHandlers.start(widget, {touches:pointers.slice(0)});
	 }

	 function pointerUp(event) {
	    if (!pointerActive) return;
	    if (!isTouchEvent(event)) return;
	    event.preventDefault();
	    var i;
	    for (i = 0; i < pointers.length; ++i) {
	       if (pointers[i].pointerId === event.pointerId) {
		  pointers.splice(i, 1);
		  break;
	       }
	    }

	    updatePointerActive();
	    touchHandlers.end(widget, {touches:pointers.slice(0),changedTouches:[]});
	 }

	 function pointerMove(event) {
	    if (!isTouchEvent(event)) return;
	    event.preventDefault();
	    var i;
	    for (i = 0; i < pointers.length; ++i) {
	       if (pointers[i].pointerId === event.pointerId) {
		  pointers[i] = event;
		  break;
	       }
	    }

	    updatePointerActive();
	    touchHandlers.moved(widget, {touches:pointers.slice(0)});
	 }

	 var o = jQuery.data(widget, 'eobj');
	 if (o) {
	    if (!window.PointerEvent) {
	       widget.removeEventListener('MSPointerDown', o.pointerDown);
	       widget.removeEventListener('MSPointerUp', o.pointerUp);
	       widget.removeEventListener('MSPointerOut', o.pointerUp);
	       widget.removeEventListener('MSPointerMove', o.pointerMove);
	    } else {
	       widget.removeEventListener('pointerdown', o.pointerDown);
	       widget.removeEventListener('pointerup', o.pointerUp);
	       widget.removeEventListener('pointerout', o.pointerUp);
	       widget.removeEventListener('pointermove', o.pointerMove);
	    }
	 }
	 jQuery.data(widget, 'eobj', {
	    pointerDown: pointerDown,
	    pointerUp: pointerUp,
	    pointerMove: pointerMove
	 });
	 if (!window.PointerEvent) {
	    widget.addEventListener('MSPointerDown', pointerDown);
	    widget.addEventListener('MSPointerUp', pointerUp);
	    widget.addEventListener('MSPointerOut', pointerUp);
	    widget.addEventListener('MSPointerMove', pointerMove);
	 } else {
	    widget.addEventListener('pointerdown', pointerDown);
	    widget.addEventListener('pointerup', pointerUp);
	    widget.addEventListener('pointerout', pointerUp);
	    widget.addEventListener('pointermove', pointerMove);
	 }
      })();
   }

   // Constants
   var FRICTION_FACTOR = 0.003, // Determines how strongly the speed decreases, when animating
       SPRING_CONSTANT = 0.0002, // How strongly the spring pulls, relative to how extended it is
       RESISTANCE_FACTOR = 0.07, // How strongly the spring resists movement, when dragging
       BOUNDS_SLACK = 3, // The amount of slack to apply to determine whether an area is within bounds
       MIN_SPEED = 0.001, // The minimum speed that the animation should pan at when out of bounds,
			  // ensures that the animation does not stop prematurely.
       MAX_SPEED = 1.5, // The maximum speed that we should cap to, to prevent glitchy stuff
       STOPPING_SPEED = 0.02; // If the speed is below the stopping speed, and we're inside of bounds,
			      // then we can stop the animation.

   jQuery.data(widget, 'cobj', this);

   var self = this;
   var WT = APP.WT;

   self.config = config;

   var overlay = jQuery.data(widget, 'oobj');

   var crosshair = null;

   var paintEnabled = true; 
   var dragPreviousXY = null;
   var touches = [];
   var singleTouch = false;
   var doubleTouch = false;
   var zoomAngle = null;
   var zoomMiddle = null;
   var zoomProjection = null;

   var v = {x: 0, y: 0};

   var lastDate = null;

   var mode = null;

   var utils = WT.gfxUtils;
   var mult = utils.transform_mult;
   var inverted = utils.transform_inverted;
   var assign = utils.transform_assign;
   var top = utils.rect_top;
   var bottom = utils.rect_bottom;
   var left = utils.rect_left;
   var right = utils.rect_right;

   var animating = false;

   function transform(ax) {
      if (ax === X) return config.xTransform;
      if (ax === Y) return config.yTransform;
   }

   function combinedTransform() {
      if (config.isHorizontal) {
	 var l = left(config.area);
	 var t = top(config.area);
	 return mult([0,1,1,0,l,t], mult(transform(X), mult(transform(Y), [0,1,1,0,-t,-l])));
      } else {
	 var l = left(config.area);
	 var b = bottom(config.area);
	 return mult([1,0,0,-1,l,b], mult(transform(X), mult(transform(Y), [1,0,0,-1,-l,b])));
      }
   }
   target.combinedTransform = combinedTransform;

   function transformedChartArea() {
      return mult(combinedTransform(), config.area);
   }

   function toModelCoord(p, noTransform) {
      if (noTransform === undefined) noTransform = false;
      var res;
      if (noTransform) {
	 res = p;
      } else {
	 res = mult(inverted(combinedTransform()), p);
      }
      var u;
      if (config.isHorizontal) {
	 u = [(res[Y] - config.area[1]) / config.area[3],
	      (res[X] - config.area[0]) / config.area[2]];
      } else {
         u = [(res[X] - config.area[0]) / config.area[2],
	       1 - (res[Y] - config.area[1]) / config.area[3]];
      }
      return [config.modelArea[0] + u[X] * config.modelArea[2],
	      config.modelArea[1] + u[Y] * config.modelArea[3]];
   }

   function toDisplayCoord(p, noTransform) {
      if (noTransform === undefined) noTransform = false;
      var u, res;
      if (config.isHorizontal) {
	 u = [(p[X] - config.modelArea[0]) / config.modelArea[2],
	      (p[Y] - config.modelArea[1]) / config.modelArea[3]];
	 res = [config.area[0] + u[Y] * config.area[2],
	        config.area[1] + u[X] * config.area[3]];
      } else {
	 u = [(p[X] - config.modelArea[0]) / config.modelArea[2],
	       1 - (p[Y] - config.modelArea[1]) / config.modelArea[3]];
	 res = [config.area[0] + u[X] * config.area[2],
	        config.area[1] + u[Y] * config.area[3]];
      }
      if (noTransform) {
	 return res;
      } else {
	 return mult(combinedTransform(), res);
      }
   }

   function findClosestPoint(x, series) {
      var axis = X;
      if (config.isHorizontal) {
	 axis = Y;
      }
      var i = binarySearch(x, series);
      if (i < 0) i = 0;
      if (i >= series.length) i = series.length - 2;
      if (series[i][axis] === x) return [series[i][X],series[i][Y]];
      var next_i = i+1;
      if (series[next_i][2] == CUBIC_C1) next_i += 2;
      var d1 = x - series[i][axis];
      var d2 = series[next_i][axis] - x;
      if (d1 < d2) {
	 return [series[i][X],series[i][Y]];
      } else {
	 return [series[next_i][X],series[next_i][Y]];
      }
   }

   // Find the anchor point (not a curve control point!)
   // with X coordinate nearest to the given x,
   // and smaller than the given x, and return
   // its index in the given series.
   function binarySearch(x, series) {
      var axis = X;
      if (config.isHorizontal) axis = Y;
      // Move back to a non-control point.
      function moveBack(i) {
	 if (series[i][2] === CUBIC_C2) --i;
	 if (series[i][2] === CUBIC_C1) --i;
	 return i;
      }
      var len = series.length;
      var i = Math.floor(len / 2);
      i = moveBack(i);
      var lower_bound = 0;
      var upper_bound = len;
      var found = false;
      if (series[0][axis] > x) return -1;
      if (series[len-1][axis] < x) return len;
      while (!found) {
	 var next_i = i + 1;
	 if (series[next_i][2] === CUBIC_C1) {
	    next_i += 2;
	 }
	 if (series[i][axis] > x) {
	    upper_bound = i;
	    i = Math.floor((upper_bound + lower_bound) / 2);
	    i = moveBack(i);
	 } else {
	    if (series[i][axis] === x) {
	       found = true;
	    } else {
	       if (series[next_i][axis] > x) {
		  found = true;
	       } else if (series[next_i][axis] === x) {
		  i = next_i;
		  found = true;
	       } else {
		  lower_bound = i;
		  i = Math.floor((upper_bound + lower_bound) / 2);
		  i = moveBack(i);
	       }
	    }
	 }
      }
      return i;
   }

   function notifyAreaChanged() {
      var u,v;
      if (config.isHorizontal) {
	 u = (toModelCoord([0, top(config.area)])[0] - config.modelArea[0]) / config.modelArea[2];
	 v = (toModelCoord([0, bottom(config.area)])[0] - config.modelArea[0]) / config.modelArea[2];
      } else {
	 u = (toModelCoord([left(config.area), 0])[0] - config.modelArea[0]) / config.modelArea[2];
	 v = (toModelCoord([right(config.area), 0])[0] - config.modelArea[0]) / config.modelArea[2];
      }
      var i;
      for (i = 0; i < config.sliders.length; ++i) {
	 var o = $('#' + config.sliders[i]);
	 if (o) {
	    var sobj = o.data('sobj');
	    if (sobj) {
	       sobj.changeRange(u, v);
	    }
	 }
      }
   }

   function repaint() {
      if (!paintEnabled) return;
      rqAnimFrame(function(){
	 target.repaint();
	 if (showCrosshair()) {
	    repaintOverlay();
	 }
      });
   }

   function repaintOverlay() {
      if (!paintEnabled) return;
      var ctx = overlay.getContext('2d');

      ctx.clearRect(0, 0, overlay.width, overlay.height);

      ctx.save();

      ctx.beginPath();
      ctx.moveTo(left(config.area), top(config.area));
      ctx.lineTo(right(config.area), top(config.area));
      ctx.lineTo(right(config.area), bottom(config.area));
      ctx.lineTo(left(config.area), bottom(config.area));
      ctx.closePath();
      ctx.clip();

      var p = mult(inverted(combinedTransform()), crosshair);
      var x = crosshair[X];
      var y = crosshair[Y];
      if (config.followCurve !== -1) {
	 p = findClosestPoint(config.isHorizontal ? p[Y] : p[X], config.series[config.followCurve]);
	 var tp = mult(combinedTransform(), p);
	 x = tp[X];
	 y = tp[Y];
	 crosshair[X] = x;
	 crosshair[Y] = y;
      }
      var u;
      if (config.isHorizontal) {
	 u = [(p[Y] - config.area[1]) / config.area[3],
	       (p[X] - config.area[0]) / config.area[2]];
      } else {
	 u = [(p[X] - config.area[0]) / config.area[2],
	       1 - (p[Y] - config.area[1]) / config.area[3]];
      }
      p = [config.modelArea[0] + u[X] * config.modelArea[2],
	   config.modelArea[1] + u[Y] * config.modelArea[3]];

      ctx.font = '16px sans-serif';
      ctx.textAlign = 'right';
      ctx.textBaseline = 'top';
      var textX = p[0].toFixed(2);
      var textY = p[1].toFixed(2);
      if (textX == '-0.00') textX = '0.00';
      if (textY == '-0.00') textY = '0.00';
      ctx.fillText("("+textX+","+textY+")", right(config.area) - config.coordinateOverlayPadding[0],
	    top(config.area) + config.coordinateOverlayPadding[1]);
      
      if (ctx.setLineDash) {
	 ctx.setLineDash([1,2]);
      }
      ctx.beginPath();
      ctx.moveTo(Math.floor(x) + 0.5, Math.floor(top(config.area)) + 0.5);
      ctx.lineTo(Math.floor(x) + 0.5, Math.floor(bottom(config.area)) + 0.5);
      ctx.moveTo(Math.floor(left(config.area)) + 0.5, Math.floor(y) + 0.5);
      ctx.lineTo(Math.floor(right(config.area)) + 0.5, Math.floor(y) + 0.5);
      ctx.stroke();

      ctx.restore();
   }

   // Check if a point is inside of the given rect
   function isPointInRect(point, rect) {
     var x,y;
     if (point.x !== undefined) {
       x = point.x;
       y = point.y;
     } else {
       x = point[0];
       y = point[1];
     }
     return x >= left(rect) && x <= right(rect) &&
	    y >= top(rect) && y <= bottom(rect);
   }

   // Check if the given area is within the bounds of the chart's area + some slack.
   function isWithinBounds(area) {
     return top(area) <= top(config.area) + BOUNDS_SLACK &&
	    bottom(area) >= bottom(config.area) - BOUNDS_SLACK &&
	    left(area) <= left(config.area) + BOUNDS_SLACK &&
	    right(area) >= right(config.area) - BOUNDS_SLACK;
   }

   function enforceLimits(flags) {
     var newChartArea = transformedChartArea();
     if (config.isHorizontal) {
	if (flags === X_ONLY) flags = Y_ONLY;
	else if (flags === Y_ONLY) flags = X_ONLY;
     }
     var diff;
     if (flags === undefined || flags === X_ONLY) {
	if (transform(X)[0] < 1) {
	   transform(X)[0] = 1;
	   newChartArea = transformedChartArea();
	}
     }
     if (flags === undefined || flags === Y_ONLY) {
	if (transform(Y)[3] < 1) {
	   transform(Y)[3] = 1;
	   newChartArea = transformedChartArea();
	}
     }
     if (flags === undefined || flags === X_ONLY) {
	if (left(newChartArea) > left(config.area)) {
	   diff = left(config.area) - left(newChartArea);
	   if (config.isHorizontal)
	      transform(Y)[5] = transform(Y)[5] + diff;
	   else
	      transform(X)[4] = transform(X)[4] + diff;
	   newChartArea = transformedChartArea();
	}
	if (right(newChartArea) < right(config.area)) {
	   diff = right(config.area) - right(newChartArea);
	   if (config.isHorizontal)
	      transform(Y)[5] = transform(Y)[5] + diff;
	   else
	      transform(X)[4] = transform(X)[4] + diff;
	   newChartArea = transformedChartArea();
	}
     }
     if (flags === undefined || flags === Y_ONLY) {
	if (top(newChartArea) > top(config.area)) {
	   diff = top(config.area) - top(newChartArea);
	   if (config.isHorizontal)
	      transform(X)[4] = transform(X)[4] + diff;
	   else
	      transform(Y)[5] = transform(Y)[5] - diff;
	   newChartArea = transformedChartArea();
	}
	if (bottom(newChartArea) < bottom(config.area)) {
	   diff = bottom(config.area) - bottom(newChartArea);
	   if (config.isHorizontal)
	      transform(X)[4] = transform(X)[4] + diff;
	   else
	      transform(Y)[5] = transform(Y)[5] - diff;
	   newChartArea = transformedChartArea();
	}
     }
   }

   this.mouseMove = function(o, event) {
      // Delay mouse move, because IE reacts to
      // mousemove first, but we actually want to
      // handle it after pointer events.
      setTimeout(function() {
	 if (pointerActive) return;
	 var c = WT.widgetCoordinates(target.canvas, event);
	 if (!isPointInRect(c, config.area)) return;

	 if (showCrosshair() && paintEnabled) {
	    crosshair = [c.x,c.y];
	    rqAnimFrame(repaintOverlay);
	 }
      }, 0);
   }

   this.mouseDown = function(o, event) {
      if (pointerActive) return;
      var c = WT.widgetCoordinates(target.canvas, event);
      if (!isPointInRect(c, config.area)) return;

      dragPreviousXY = c;
   };

   this.mouseUp = function(o, event) {
      if (pointerActive) return;
      dragPreviousXY = null;
   };

   this.mouseDrag = function(o, event) {
      if (pointerActive) return;
      if (dragPreviousXY === null) return;
      var c = WT.widgetCoordinates(target.canvas, event);
      if (!isPointInRect(c, config.area)) return;
      if (WT.buttons === 1) {
	 if (config.pan) {
	    translate({
	       x: c.x - dragPreviousXY.x,
	       y: c.y - dragPreviousXY.y
	    });
	 }
      }
      dragPreviousXY = c;
   };

   function init() {
      if (showCrosshair && (overlay === undefined || target.canvas.width !== overlay.width || target.canvas.height !== overlay.height)) {
	 if (overlay) {
	    overlay.parentNode.removeChild(overlay);
	    jQuery.removeData(widget, 'oobj');
	    overlay = undefined;
	 }
	 c = document.createElement("canvas");
	 c.setAttribute("width", target.canvas.width);
	 c.setAttribute("height", target.canvas.height);
	 c.style.position = 'absolute';
	 c.style.display = 'block';
	 c.style.left = '0';
	 c.style.top = '0';
	 if (window.MSPointerEvent || window.PointerEvent) {
	    c.style.msTouchAction = 'none';
	    c.style.touchAction = 'none';
	 }
	 target.canvas.parentNode.appendChild(c);
	 overlay = c;
	 jQuery.data(widget, 'oobj', overlay);
      } else if (overlay !== undefined && !showCrosshair()) {
	 // If the mouse handler is not reinitialized, we don't actually get here!
	 overlay.parentNode.removeChild(overlay);
	 jQuery.removeData(widget, 'oobj');
	 overlay = undefined;
      }

      if (crosshair === null) {
	 crosshair = toDisplayCoord([(left(config.modelArea) + right(config.modelArea)) / 2,
				     (top(config.modelArea) + bottom(config.modelArea)) / 2]);
      }
   }

   this.mouseWheel = function(o, event) {
      var modifiers = (event.metaKey << 3) + (event.altKey << 2) + (event.ctrlKey << 1) + event.shiftKey;
      var action = config.wheelActions[modifiers];
      if (action === undefined) return;

      var c = WT.widgetCoordinates(target.canvas, event);
      if (!isPointInRect(c, config.area)) return;
      var w = WT.normalizeWheel(event);
      if ((action === WHEEL_PAN_X || action === WHEEL_PAN_Y || action === WHEEL_PAN_MATCHING) && config.pan) {
	 var xBefore = transform(X)[4];
	 var yBefore = transform(Y)[5];
	 if (action === WHEEL_PAN_MATCHING)
	    translate({x:-w.pixelX,y:-w.pixelY});
	 else if (action === WHEEL_PAN_Y)
	    translate({x:0,y:-w.pixelX - w.pixelY});
	 else if (action === WHEEL_PAN_X)
	    translate({x:-w.pixelX - w.pixelY,y:0});
	 if (xBefore !== transform(X)[4] ||
	     yBefore !== transform(Y)[5]) {
	    WT.cancelEvent(event);
	 }
      } else if (config.zoom) {
	 WT.cancelEvent(event);
	 var d = -w.spinY;
	 // Some browsers scroll horizontally when shift key pressed
	 if (d === 0) d = -w.spinX;
	 if (action === WHEEL_ZOOM_Y) {
	    zoom(c, 0, d);
	 } else if (action === WHEEL_ZOOM_X) {
	    zoom(c, d, 0);
	 } else if (action === WHEEL_ZOOM_XY) {
	    zoom(c, d, d);
	 } else if (action === WHEEL_ZOOM_MATCHING) {
	    if (w.pixelX !== 0)
	       zoom(c, d, 0);
	    else
	       zoom(c, 0, d);
	 }
      }
   };

   // Get projection matrix to project any point
   // to a line through m at angle theta
   function projection(theta, m) {
      var c = Math.cos(theta);
      var s = Math.sin(theta);
      var c2 = c*c;
      var s2 = s*s;
      var cs = c*s;
      var h = -m[0]*c - m[1]*s;
      return [c2, cs, cs, s2, c*h+m[0], s*h+m[1]];
   }

   function distanceLessThanRadius(p1, p2, radius) {
      var d = [p2[X] - p1[X],
	       p2[Y] - p1[Y]];
      return radius * radius >= d[X] * d[X] + d[Y] * d[Y];
   }

   var CROSSHAIR_RADIUS = 30;

   touchHandlers.start = function(o, event) {
      singleTouch = event.touches.length === 1;
      doubleTouch = event.touches.length === 2;

      if (singleTouch) {
	 animating = false;
	 var c = WT.widgetCoordinates(target.canvas, event.touches[0]);
	 if (!isPointInRect(c, config.area)) return;
	 if (showCrosshair() && distanceLessThanRadius(crosshair, [c.x,c.y], CROSSHAIR_RADIUS)) {
	    mode = CROSSHAIR_MODE;
	 } else {
	    mode = LOOK_MODE;
	 }
	 lastDate = Date.now();
	 dragPreviousXY = c;
	 WT.capture(null);
	 WT.capture(target.canvas);
      } else if (doubleTouch && config.zoom) {
	 animating = false;
	touches = [
	   WT.widgetCoordinates(target.canvas,event.touches[0]),
	   WT.widgetCoordinates(target.canvas,event.touches[1])
	].map(function(t){return [t.x,t.y];});
	if (!touches.every(function(p){return isPointInRect(p,config.area);})) {
	   doubleTouch = null;
	   return;
	}
	WT.capture(null);
	WT.capture(target.canvas);
	zoomAngle = Math.atan2(touches[1][1] - touches[0][1], touches[1][0] - touches[0][0]);
	zoomMiddle = [
	   (touches[0][0] + touches[1][0]) / 2,
	   (touches[0][1] + touches[1][1]) / 2];
	var sin = Math.abs(Math.sin(zoomAngle));
	var cos = Math.abs(Math.cos(zoomAngle));
	if (sin < Math.sin(22.5 / 180 * Math.PI)) {
	   zoomAngle = 0;
	} else if (cos < Math.cos(67.5 / 180 * Math.PI)) {
	   zoomAngle = Math.PI / 2;
	} else if (Math.tan(zoomAngle) > 0) {
	   zoomAngle = Math.PI / 4;
	} else {
	   zoomAngle = -Math.PI / 4;
	}
	zoomProjection = projection(zoomAngle, zoomMiddle);
      } else {
	 return;
      }
      if (event.preventDefault) event.preventDefault();
   };

   function animate(ts, dt) {
      if (!animating) return;
      var now = Date.now();
      if (dt === undefined) {
	 dt = now - lastDate;
      }
      var d = {x: 0, y: 0};
      var area = transformedChartArea();
      var k = SPRING_CONSTANT;

      if (dt > 2 * ANIMATION_INTERVAL) {
	 paintEnabled = false;
	 var i = Math.floor(dt / ANIMATION_INTERVAL - 1);
	 var j;
	 for (j = 0; j < i; ++j) {
	    animate(ts, ANIMATION_INTERVAL);
	    if (!animating) {
	       paintEnabled = true;
	       repaint();
	       return;
	    }
	 }
	 dt -= i * ANIMATION_INTERVAL;
	 paintEnabled = true;
      }

      // Calculate new area position and v.x, v.y
      if (v.x === Infinity || v.x === -Infinity) {
	 if (v.x > 0) v.x = MAX_SPEED;
	 else v.x = -MAX_SPEED;
      }
      if (isFinite(v.x)) {
	 v.x = v.x / (1 + FRICTION_FACTOR * dt);
	 area[0] += v.x * dt;
	 if (left(area) > left(config.area)) {
	    v.x = v.x + (-k) * (left(area) - left(config.area)) * dt;
	    v.x *= 0.7;
	 } else if (right(area) < right(config.area)) {
	    v.x = v.x + (-k) * (right(area) - right(config.area)) * dt;
	    v.x *= 0.7;
	 }
	 if (Math.abs(v.x) < MIN_SPEED) {
	    if (left(area) > left(config.area)) {
	       v.x = MIN_SPEED;
	    } else if (right(area) < right(config.area)) {
	       v.x = -MIN_SPEED;
	    }
	 }
	 // cap speed
	 if (Math.abs(v.x) > MAX_SPEED) v.x = (v.x > 0 ? 1 : -1) * MAX_SPEED;
	 d.x = v.x * dt;
      }
      if (v.y === Infinity || v.y === -Infinity) {
	 if (v.y > 0) v.y = MAX_SPEED;
	 else v.y = -MAX_SPEED;
      }
      if (isFinite(v.y)) {
	 v.y = v.y / (1 + FRICTION_FACTOR * dt);
	 area[1] += v.y * dt;
	 if (top(area) > top(config.area)) {
	    v.y = v.y + (-k) * (top(area) - top(config.area)) * dt;
	    v.y *= 0.7;
	 } else if (bottom(area) < bottom(config.area)) {
	    v.y = v.y + (-k) * (bottom(area) - bottom(config.area)) * dt;
	    v.y *= 0.7;
	 }
	 if (Math.abs(v.y) < 0.001) {
	    if (top(area) > top(config.area)) {
	       v.y = 0.001;
	    } else if (bottom(area) < bottom(config.area)) {
	       v.y = -0.001;
	    }
	 }
	 // cap speed
	 if (Math.abs(v.y) > MAX_SPEED) v.y = (v.y > 0 ? 1 : -1) * MAX_SPEED;
	 d.y = v.y * dt;
      }

      area = transformedChartArea();
      translate(d, NO_LIMIT);
      var newArea = transformedChartArea();
      if (left(area) > left(config.area) &&
	  left(newArea) <= left(config.area)) {
	v.x = 0;
	translate({x:-d.x,y:0}, NO_LIMIT);
	enforceLimits(X_ONLY);
      }
      if (right(area) < right(config.area) &&
	  right(newArea) >= right(config.area)) {
	v.x = 0;
	translate({x:-d.x,y:0}, NO_LIMIT);
	enforceLimits(X_ONLY);
      }
      if (top(area) > top(config.area) &&
	  top(newArea) <= top(config.area)) {
	v.y = 0;
	translate({x:0,y:-d.y}, NO_LIMIT);
	enforceLimits(Y_ONLY);
      }
      if (bottom(area) < bottom(config.area) &&
	  bottom(newArea) >= bottom(config.area)) {
	v.y = 0;
	translate({x:0,y:-d.y}, NO_LIMIT);
	enforceLimits(Y_ONLY);
      }
      if (Math.abs(v.x) < STOPPING_SPEED &&
	  Math.abs(v.y) < STOPPING_SPEED &&
	  isWithinBounds(newArea)) {
	 enforceLimits();
	 animating = false;
	 dragPreviousXY = null;
	 v.x = 0;
      // Calculate new area p
	 v.y = 0;
	 lastDate = null;
	 touches = [];
      } else {
	 lastDate = now;
	 if (paintEnabled) {
	    rqAnimFrame(animate);
	 }
      }
   }

   touchHandlers.end = function(o, event) {
      var touches = Array.prototype.slice.call(event.touches);

      var noTouch = touches.length === 0;
      singleTouch = touches.length === 1;
      doubleTouch = touches.length === 2;

      if (!noTouch) {
	 (function(){
	    var i;
	    for (i = 0; i < event.changedTouches.length; ++i) {
	       (function(){
		  var id = event.changedTouches[i].identifier;
		  for (var j = 0; j < touches.length; ++j) {
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

      if (noTouch) {
	 if (mode === LOOK_MODE && (isFinite(v.x) || isFinite(v.y)) && config.rubberBand) {
	    lastDate = Date.now();
	    animating = true;
	    rqAnimFrame(animate);
	 } else {
	    self.mouseUp(null, null);
	    touches = [];
	    zoomAngle = null;
	    zoomMiddle = null;
	    zoomProjection = null;
	    if (lastDate != null) {
	       var now = Date.now();
	       lastDate = null;
	    }
	 }
	 mode = null;
      } else if (singleTouch || doubleTouch)
	 touchHandlers.start(o, event);
   };

   touchHandlers.moved = function(o, event) {
     if ( (!singleTouch) && (!doubleTouch) ) {
       return;
     }

     if (singleTouch) {
	if (dragPreviousXY === null) return;
	var c = WT.widgetCoordinates(target.canvas, event.touches[0]);
	var now = Date.now();
	var d = {
	   x: c.x - dragPreviousXY.x,
	   y: c.y - dragPreviousXY.y
	};
	var dt = now - lastDate;
	lastDate = now;
	if (mode === CROSSHAIR_MODE) {
	   crosshair[X] += d.x;
	   crosshair[Y] += d.y;
	   if (showCrosshair() && paintEnabled) {
	      rqAnimFrame(repaintOverlay);
	   }
	} else if (config.pan) {
	   if (c.x < config.area[0] || c.x > config.area[0] + config.area[2]) {
	      v = {x:0,y:0};
	      return;
	   }
	   if (c.y < config.area[1] || c.y > config.area[1] + config.area[3]) {
	      v = {x:0,y:0};
	      return;
	   }
	   v.x = d.x / dt;
	   v.y = d.y / dt;
	   translate(d, config.rubberBand ? DAMPEN : 0);
	}
	if (event.preventDefault) event.preventDefault();
	dragPreviousXY = c;
     } else if (doubleTouch && config.zoom) {
	if (event.preventDefault) event.preventDefault();
	var crosshairBefore = toModelCoord(crosshair);
	var mxBefore = (touches[0][0] + touches[1][0]) / 2;
	var myBefore = (touches[0][1] + touches[1][1]) / 2;
	var newTouches = [
	   WT.widgetCoordinates(target.canvas,event.touches[0]),
	   WT.widgetCoordinates(target.canvas,event.touches[1])
	].map(function(t){
	   if (zoomAngle === 0) {
	      return [t.x, myBefore];
	   } else if (zoomAngle === Math.PI / 2) {
	      return [mxBefore, t.y];
	   } else {
	      return mult(zoomProjection,[t.x,t.y]);
	   }
	});

	var dxBefore = Math.abs(touches[1][0] - touches[0][0]);
	var dxAfter = Math.abs(newTouches[1][0] - newTouches[0][0]);
	var xScale = dxBefore > 0 ? dxAfter / dxBefore : 1;
	if (dxAfter === dxBefore || zoomAngle === Math.PI / 2) {
	   xScale = 1;
	}
	var mxAfter = (newTouches[0][0] + newTouches[1][0]) / 2;
	var dyBefore = Math.abs(touches[1][1] - touches[0][1]);
	var dyAfter = Math.abs(newTouches[1][1] - newTouches[0][1]);
	var yScale = dyBefore ? dyAfter / dyBefore : 1;
	if (dyAfter === dyBefore || zoomAngle === 0) {
	   yScale = 1;
	}
	var myAfter = (newTouches[0][1] + newTouches[1][1]) / 2;

	if (config.isHorizontal) {
	   (function() {
	     var tmp = xScale;
	     xScale = yScale;
	     yScale = tmp;
	     tmp = mxAfter;
	     mxAfter = myAfter;
	     myAfter = tmp;
	     tmp = mxBefore;
	     mxBefore = myBefore;
	     myBefore = tmp;
	   })();
	}

	if (transform(X)[0] * xScale > config.maxZoom[X]) {
	   xScale = config.maxZoom[X] / transform(X)[0];
	}
	if (transform(Y)[3] * yScale > config.maxZoom[Y]) {
	   yScale = config.maxZoom[Y] / transform(Y)[3];
	}
	if (xScale !== 1 &&
	      (xScale < 1.0 || transform(X)[0] !== config.maxZoom[X])) {
	   assign(transform(X),
	     mult(
		[xScale,0,0,1,-xScale*mxBefore+mxAfter,0],
		transform(X)
	      )
	   );
	}
	if (yScale !== 1 &&
	      (yScale < 1.0 || transform(Y)[3] !== config.maxZoom[Y])) {
	   assign(transform(Y),
	     mult(
		[1,0,0,yScale,0,-yScale*myBefore+myAfter],
		transform(Y)
	      )
	   );
	}
	enforceLimits();

        var crosshairAfter = toDisplayCoord(crosshairBefore);
	crosshair[X] = crosshairAfter[X];
	crosshair[Y] = crosshairAfter[Y];

	touches = newTouches;
	refreshPenColors();
	repaint();
	notifyAreaChanged();
     }
   };

   function toZoomLevel(zoomFactor) {
      return Math.floor(Math.log(zoomFactor) / Math.LN2 + 0.5) + 1;
   }

   function refreshPenColors() {
      var i, j;
      var xLevel = toZoomLevel(transform(X)[0]) - 1;
      if (xLevel >= config.pens.x.length) xLevel = config.pens.x.length - 1;
      for (i = 0; i < config.pens.x.length; ++i) {
	 if (xLevel === i) {
	    for (j = 0; j < config.pens.x[i].length; ++j) {
	       config.pens.x[i][j].color[3] = config.penAlpha.x[j];
	    }
	 } else {
	    for (j = 0; j < config.pens.x[i].length; ++j) {
	       config.pens.x[i][j].color[3] = 0;
	    }
	 }
      }
      var yLevel = toZoomLevel(transform(Y)[3]) - 1;
      if (yLevel >= config.pens.y.length) yLevel = config.pens.y.length - 1;
      for (i = 0; i < config.pens.y.length; ++i) {
	 if (yLevel === i) {
	    for (j = 0; j < config.pens.y[i].length; ++j) {
	       config.pens.y[i][j].color[3] = config.penAlpha.y[j];
	    }
	 } else {
	    for (j = 0; j < config.pens.y[i].length; ++j) {
	       config.pens.y[i][j].color[3] = 0;
	    }
	 }
      }
   }

   function translate(d, flags) {
      var crosshairBefore = toModelCoord(crosshair);

      if (config.isHorizontal) {
	 d = {x:d.y,y:-d.x};
      }

      if (flags & NO_LIMIT) {
	 transform(X)[4] = transform(X)[4] + d.x;
	 transform(Y)[5] = transform(Y)[5] - d.y;
      } else if (flags & DAMPEN) {
	 var area = transformedChartArea();
	 if (left(area) > left(config.area)) {
	    if (d.x > 0) {
	       d.x = d.x / (1 + ((left(area) - left(config.area)) * RESISTANCE_FACTOR));
	    }
	 } else if (right(area) < right(config.area)) {
	    if (d.x < 0) {
	       d.x = d.x / (1 + ((right(config.area) - right(area)) * RESISTANCE_FACTOR));
	    }
	 }
	 if (top(area) > top(config.area)) {
	    if (d.y > 0) {
	       d.y = d.y / (1 + ((top(area) - top(config.area)) * RESISTANCE_FACTOR));
	    }
	 } else if (bottom(area) < bottom(config.area)) {
	    if (d.y < 0) {
	       d.y = d.y / (1 + ((bottom(config.area) - bottom(area)) * RESISTANCE_FACTOR));
	    }
	 }
	 transform(X)[4] = transform(X)[4] + d.x;
	 transform(Y)[5] = transform(Y)[5] - d.y;
	 crosshair[X] = crosshair[X] + d.x;
	 crosshair[Y] = crosshair[Y] + d.y;
      } else {
	 transform(X)[4] = transform(X)[4] + d.x;
	 transform(Y)[5] = transform(Y)[5] - d.y;
	 crosshair[X] = crosshair[X] + d.x;
	 crosshair[Y] = crosshair[Y] + d.y;

	 enforceLimits();
      }

      var crosshairAfter = toDisplayCoord(crosshairBefore);

      crosshair[X] = crosshairAfter[X];
      crosshair[Y] = crosshairAfter[Y];

      repaint();
      notifyAreaChanged();
   }

   function zoom(coords, xDelta, yDelta) {
      var crosshairBefore = toModelCoord(crosshair);
      var xy;
      if (config.isHorizontal) {
	 xy = [coords.y - top(config.area), coords.x - left(config.area)];
      } else {
	 xy = mult(
	    inverted([1,0,0,-1,left(config.area),bottom(config.area)]), [coords.x, coords.y]);
      }
      var x = xy[0];
      var y = xy[1];
      var s_x = Math.pow(1.2, config.isHorizontal ? yDelta : xDelta);
      var s_y = Math.pow(1.2, config.isHorizontal ? xDelta : yDelta);
      if (transform(X)[0] * s_x > config.maxZoom[X]) {
	 s_x = config.maxZoom[X] / transform(X)[0];
      }
      if (s_x < 1.0 || transform(X)[0] !== config.maxZoom[X]) {
	 assign(transform(X),
	       mult(
		  [s_x,0,0,1,x-s_x*x,0],
		  transform(X)));
      }
      if (transform(Y)[3] * s_y > config.maxZoom[Y]) {
	 s_y = config.maxZoom[Y] / transform(Y)[3];
      }
      if (s_y < 1.0 || transform(Y)[3] !== config.maxZoom[Y]) {
	 assign(transform(Y),
	       mult(
		  [1,0,0,s_y,0,y-s_y*y],
		  transform(Y)));
      }

      enforceLimits();

      var crosshairAfter = toDisplayCoord(crosshairBefore);
      crosshair[X] = crosshairAfter[X];
      crosshair[Y] = crosshairAfter[Y];

      refreshPenColors();
      repaint();
      notifyAreaChanged();
   }

   this.setXRange = function(seriesNb, lowerBound, upperBound) {
      lowerBound = config.modelArea[0] + config.modelArea[2] * lowerBound;
      upperBound = config.modelArea[0] + config.modelArea[2] * upperBound;
      if (lowerBound < left(config.modelArea)) lowerBound = left(config.modelArea);
      if (upperBound > right(config.modelArea)) upperBound = right(config.modelArea);
      // Set X range, and adjust Y!
      var series = config.series[seriesNb];
      if (series.length === 0) return; // This would be weird?
      var p0 = toDisplayCoord([lowerBound, 0], true);
      var p1 = toDisplayCoord([upperBound, 0], true);
      var axis = config.isHorizontal ? Y : X;
      var otherAxis = config.isHorizontal ? X : Y;
      var i0 = binarySearch(p0[axis], series);
      if (i0 < 0) {
	 i0 = 0;
      } else {
	 i0 ++;
	 if (series[i0][2] === CUBIC_C1) i0 += 2;
      }
      var i_n = binarySearch(p1[axis], series);
      var i, u, y, before_i0, after_i_n;
      var min_y = Infinity;
      var max_y = -Infinity;
      for (i = i0; i <= i_n && i < series.length; ++i) {
	 if (series[i][2] !== CUBIC_C1 && series[i][2] !== CUBIC_C2) {
	    if (series[i][otherAxis] < min_y) min_y = series[i][otherAxis];
	    if (series[i][otherAxis] > max_y) max_y = series[i][otherAxis];
	 }
      }
      if (i0 > 0) {
	 // Interpolate on the lower X end
	 before_i0 = i0 - 1;
	 if (series[before_i0][2] === CUBIC_C2) before_i0 -= 2;
	 u = (p0[axis] - series[before_i0][axis]) / (series[i0][axis] - series[before_i0][axis]);
	 y = series[before_i0][otherAxis] + u * (series[i0][otherAxis] - series[before_i0][otherAxis]);
	 if (y < min_y) min_y = y;
	 if (y > max_y) max_y = y;
      }
      if (i_n < series.length - 1) {
	 // Interpolate on the upper X end
	 after_i_n = i_n + 1;
	 if (series[after_i_n][2] === CUBIC_C1) after_i_n += 2;
	 u = (p1[axis] - series[i_n][axis]) / (series[after_i_n][axis] - series[i_n][axis]);
	 y = series[i_n][otherAxis] + u * (series[after_i_n][otherAxis] - series[i_n][otherAxis]);
	 if (y < min_y) min_y = y;
	 if (y > max_y) max_y = y;
      }
      var xZoom = config.modelArea[2] / (upperBound - lowerBound);
      var H = config.isHorizontal ? 2 : 3;
      var yZoom = config.area[H] / (max_y - min_y);
      var yMargin = 10;
      yZoom = config.area[H] / (config.area[H] / yZoom + yMargin * 2); // Give it 10 px extra on each side
      if (yZoom > config.maxZoom[otherAxis]) yZoom = config.maxZoom[otherAxis];
      var panPoint;
      if (config.isHorizontal)
	 panPoint = [p0[Y] - top(config.area), ((min_y + max_y) / 2 - (config.area[2] / yZoom) / 2 - left(config.area))];
      else
	 panPoint = [p0[X] - left(config.area), -((min_y + max_y) / 2 + (config.area[3] / yZoom) / 2 - bottom(config.area))];

      var crosshairBefore = toModelCoord(crosshair);

      transform(X)[0] = xZoom;
      transform(Y)[3] = yZoom;
      transform(X)[4] = -panPoint[X] * xZoom;
      transform(Y)[5] = -panPoint[Y] * yZoom;

      var crosshairAfter = toDisplayCoord(crosshairBefore);
      crosshair[X] = crosshairAfter[X];
      crosshair[Y] = crosshairAfter[Y];

      enforceLimits();
      refreshPenColors();
      repaint();
      notifyAreaChanged();
   }

   this.getSeries = function(seriesNb) {
      return config.series[seriesNb];
   }

   this.rangeChangedCallbacks = [];

   this.updateConfig = function(newConfig) {
      for (var key in newConfig) {
	 if (newConfig.hasOwnProperty(key)) {
	    config[key] = newConfig[key];
	 }
      }
      init();
      refreshPenColors();
      repaint();
      notifyAreaChanged();
   }

   this.updateConfig({});

   if (window.TouchEvent && !window.MSPointerEvent && !window.PointerEvent) {
      self.touchStart = touchHandlers.start;
      self.touchEnd = touchHandlers.end;
      self.touchMoved = touchHandlers.moved;
   } else {
      var nop = function(){};
      self.touchStart = nop;
      self.touchEnd = nop;
      self.touchMoved = nop;
   }
 });
