/*
 * Copyright (C) 2015 Emweb bvba, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER
(10, JavaScriptConstructor, "WPaintedWidget",
 function(APP, widget) {
   this.canvas = document.getElementById('c' + widget.id);
   var ctx = this.canvas.getContext('2d');

   jQuery.data(widget, 'obj', this);

   var self = this;
   var WT = APP.WT;

   this.jsValues = []
   this.repaint = function() {};
   this.widget = widget;

   function encodeJSValues() {
      var res = [];
      var value;
      var i;
      for (i = 0; i < self.jsValues.length; ++i) {
	 value = self.jsValues[i];
	 if (jQuery.isArray(value) && value.length > 6) {
	    res.push([]); // Omit painter paths, FIXME: this is kind of hacky?
	 } else {
	    res.push(value);
	 }
      }
      return JSON.stringify(res);
   }
   widget.wtEncodeValue = encodeJSValues;
 });

// This should be refactored to something, somewhere?
WT_DECLARE_WT_MEMBER
(11, JavaScriptObject, "gfxUtils",
   (function() {
      var M11 = 0, M12 = 1, M21 = 2, M22 = 3, M13 = 4, M23 = 5;
      var MOVE_TO = 0, LINE_TO = 1, CUBIC_C1 = 2, CUBIC_C2 = 3, CUBIC_END = 4,
	  QUAD_C = 5, QUAD_END = 6, ARC_C = 7, ARC_R = 8, ARC_ANGLE_SWEEP = 9;
      var ALIGN_LEFT = 0x1,
	  ALIGN_RIGHT = 0x2,
	  ALIGN_CENTER = 0x4,
	  ALIGN_JUSTIFY = 0x8, // unsupporetd?
	  ALIGN_BASELINE = 0x10, // unsupported?
	  ALIGN_SUB = 0x20, // unsupported?
	  ALIGN_SUPER = 0x40, // unsupported?
	  ALIGN_TOP = 0x80,
	  ALIGN_TEXT_TOP = 0x100, // unsupported?
          ALIGN_MIDDLE = 0x200,
	  ALIGN_BOTTOM = 0x400,
	  ALIGN_TEXT_BOTTOM = 0x800; // unsupported?
      var ALIGN_VERTICAL_MASK =
            ALIGN_BASELINE | ALIGN_SUB | ALIGN_SUPER | ALIGN_TOP | ALIGN_TEXT_TOP | ALIGN_MIDDLE | ALIGN_BOTTOM | ALIGN_TEXT_BOTTOM;
      var ALIGN_HORIZONTAL_MASK =
	    ALIGN_LEFT | ALIGN_RIGHT | ALIGN_CENTER | ALIGN_JUSTIFY;

      function Utils() {
	 var self = this;

	 this.path_crisp = function(path) {
	    return path.map(
	       function(p) {
		  return [Math.floor(p[0]) + 0.5, Math.floor(p[1]) + 0.5, p[2]];
	       }
	    );
	 };
	 this.transform_mult = function(t1,t2) {
	    if (t2.length === 2) {
	       // WPointF
	       var x = t2[0], y = t2[1];
	       return [
	         t1[M11] * x + t1[M12] * y + t1[M13],
	         t1[M21] * x + t1[M22] * y + t1[M23]
	       ];
	    }
	    if (t2.length === 3) {
	       if (t2[2] === ARC_R || t2[2] === ARC_ANGLE_SWEEP) {
		  return t2.slice(0); // Don't transform radius
	       }
	       // WPainterPath component
	       var x = t2[0], y = t2[1];
	       return [
	         t1[M11] * x + t1[M12] * y + t1[M13],
	         t1[M21] * x + t1[M22] * y + t1[M23],
		 t2[2]
	       ];
	    }
	    if (t2.length === 4) {
	       // WRectF
	       var minX, minY, maxX, maxY, i, p2;
	       var p = self.transform_mult(t1, [t2[0],t2[1]]);
	       minX = p[0];
	       maxX = p[0];
	       minY = p[1];
	       maxY = p[1];
	       
	       for (i = 0; i < 3; ++i) {
		  p2 = self.transform_mult(t1,
					  i == 0 ? [self.rect_left(t2), self.rect_bottom(t2)]
			                  : i == 1 ? [self.rect_right(t2), self.rect_top(t2)]
					  : [self.rect_right(t2), self.rect_bottom(t2)]);
		  minX = Math.min(minX, p2[0]);
		  maxX = Math.max(maxX, p2[0]);
		  minY = Math.min(minY, p2[1]);
		  maxY = Math.max(maxY, p2[1]);
	       }

	       return [minX, minY, maxX - minX, maxY - minY];
	    }
	    if (t2.length === 6) {
	       // WTransform
	       return [
		  t1[M11] * t2[M11] + t1[M12] * t2[M21],
		  t1[M11] * t2[M12] + t1[M12] * t2[M22],
		  t1[M21] * t2[M11] + t1[M22] * t2[M21],
		  t1[M21] * t2[M12] + t1[M22] * t2[M22],
		  t1[M11] * t2[M13] + t1[M12] * t2[M23] + t1[M13],
		  t1[M21] * t2[M13] + t1[M22] * t2[M23] + t1[M23]
	       ];
	    }
	    return [];
	 };
	 // Apply a transform to a path
	 this.transform_apply = function(t, path) {
	    var transform_mult = self.transform_mult;
	    return path.map(function(p) {
	       return transform_mult(t, p);
	    });
	 };
	 this.transform_det = function(t) {
	    var m11 = t[M11],
	        m12 = t[M21],
		m21 = t[M12],
		m22 = t[M22];
	    return m11 * m22 - m21 * m12;
	 };
	 this.transform_adjoint = function(t) {
	    var m11 = t[M11],
	        m12 = t[M21],
		m21 = t[M12],
		m22 = t[M22],
		m31 = t[M13],
		m32 = t[M23];
	    return [  m22, - m12, - m21, m11,
		      m32 * m21 - m31 * m22,
		    - (m32 * m11 - m31 * m12) ];
	 };
	 this.transform_inverted = function(t) {
	    var det = self.transform_det(t);
	    if (det != 0) {
	       var adj = self.transform_adjoint(t);
	       var m11 = adj[M11],
		   m12 = adj[M21],
		   m21 = adj[M12],
		   m22 = adj[M22],
		   m31 = adj[M13],
		   m32 = adj[M23];

	       return [m11 / det, m12 / det,
		       m21 / det, m22 / det,
		       m31 / det, m32 / det];
	    } else {
	       console.log("inverted(): oops, determinant == 0");
	       return t;
	    }

	 };
	 this.transform_assign = function(t1, t2) {
	    t1[0] = t2[0];
	    t1[1] = t2[1];
	    t1[2] = t2[2];
	    t1[3] = t2[3];
	    t1[4] = t2[4];
	    t1[5] = t2[5];
	 };
	 this.transform_equal = function(t1, t2) {
	    return t1[0] == t2[0] &&
	           t1[1] == t2[1] &&
	           t1[2] == t2[2] &&
	           t1[3] == t2[3] &&
	           t1[4] == t2[4] &&
	           t1[5] == t2[5];
	 };
	 this.css_text = function(c) {
	    return "rgba(" + c[0] + "," + c[1] + "," + c[2] + "," + c[3] + ")";
	 };
	 this.arcPosition = function(cx, cy, rx, ry, angle) {
	    var a = -angle / 180 * Math.PI;

	    return [
	       cx + rx * Math.cos(a),
	       cy + ry * Math.sin(a)
	    ];
	 };
	 this.pnpoly = function(p, path) {
	    var res = false;
	    var ax = 0.0, ay = 0.0;
	    var px = p[0], py = p[1];
	    var i, bx, by;
	    for (i = 0; i < path.length; ++i) {
	       bx = ax;
	       by = ay;
	       if (path[i][2] === ARC_C) {
		  var arcPos = self.arcPosition(path[i][0], path[i][1],
						path[i+1][0], path[i+1][1],
						path[i+2][0]);
		  bx = arcPos[0];
		  by = arcPos[1];
	       } else if (path[i][2] === ARC_ANGLE_SWEEP) {
		  var arcPos = self.arcPosition(path[i-2][0], path[i-2][1],
						path[i-1][0], path[i-1][1],
						path[i][0] + path[i][1]);
		  bx = arcPos[0];
		  by = arcPos[1];
	       } else if (path[i][2] !== ARC_R) {
		  bx = path[i][0];
		  by = path[i][1];
	       }
	       if (path[i][2] !== MOVE_TO) {
		 if ( (ay > py) !== (by > py) &&
		      (px < (bx - ax) * (py - ay) / (by - ay) + ax) ) {
		   res = !res;
		 }
	       }
	       ax = bx;
	       ay = by;
	    }
	    return res;
	 };
	 this.drawRect = function(ctx, rect, fill, stroke) {
	    rect = self.rect_normalized(rect);
	    var t = self.rect_top(rect),
		b = self.rect_bottom(rect),
		l = self.rect_left(rect),
		r = self.rect_right(rect);
	    path = [[l, t, MOVE_TO],
		    [r, t, LINE_TO],
		    [r, b, LINE_TO],
		    [l, b, LINE_TO],
		    [l, t, LINE_TO]];
	    self.drawPath(ctx, path, fill, stroke, false);
	 };
	 this.drawPath = function(ctx, path, fill, stroke, clip) {
	    var i = 0, bezier = [], arc = [], quad = [];
	    function x(segment) { return segment[0]; }
	    function y(segment) { return segment[1]; }
	    function type(segment) { return segment[2]; }
	    ctx.beginPath();
	    if (path.length > 0 && type(path[0]) !== MOVE_TO) {
	       ctx.moveTo(0,0);
	    }
	    for (i = 0; i < path.length; i++) {
	       var s = path[i];

	       switch (type(s)) {
	       case MOVE_TO:
		  ctx.moveTo(x(s), y(s));
		  break;
	       case LINE_TO:
		  ctx.lineTo(x(s), y(s));
		  break;
	       case CUBIC_C1:
		  bezier.push(x(s), y(s));
		  break;
	       case CUBIC_C2:
		  bezier.push(x(s), y(s));
		  break;
	       case CUBIC_END:
		  bezier.push(x(s), y(s));
		  ctx.bezierCurveTo.apply(ctx, bezier);
		  bezier = [];
		  break;
	       case ARC_C:
		  arc.push(x(s), y(s));
		  break;
	       case ARC_R:
		  arc.push(x(s));
		  break;
	       case ARC_ANGLE_SWEEP:
		  arc.push((x(s) * Math.PI)/180.0, (y(s) * Math.PI)/180.0, y(s) > 0);
		  ctx.arc.apply(ctx, arc);
		  arc = [];
		  break;
	       case QUAD_C:
		  quad.push(x(s));
		  break;
	       case QUAD_END:
		  quad.push(x(s), y(s));
		  ctx.quadraticCurveTo.apply(ctx, quad);
		  quad = [];
		  break;
	       };
	    }
	    if (fill) ctx.fill();
	    if (stroke) ctx.stroke();
	    if (clip) ctx.clip();
	 };
	 this.drawStencilAlongPath = function(ctx, stencil, path, fill, stroke, softClipping) {
	    var i = 0;
	    function x(segment) { return segment[0]; }
	    function y(segment) { return segment[1]; }
	    function type(segment) { return segment[2]; }
	    for (i = 0; i < path.length; i++) {
	       var s = path[i];
	       if (softClipping &&
		 ctx.wtClipPath &&
		 !self.pnpoly(s,
		    self.transform_apply(ctx.wtClipPathTransform,
					 ctx.wtClipPath))) {
		continue;
	      }
	      if (type(s) == MOVE_TO || type(s) == LINE_TO ||
		  type(s) == QUAD_END || type(s) == CUBIC_END) {
		var translatedStencil = self.transform_apply([1,0,0,1,x(s),y(s)], stencil);
		self.drawPath(ctx, translatedStencil, fill, stroke, false);
	      }
	    }
	 };
	 this.drawText = function(ctx, rect, align, text, clipPoint) {
	   if (clipPoint &&
	       ctx.wtClipPath &&
	       !self.pnpoly(clipPoint,
		  self.transform_apply(ctx.wtClipPathTransform,
				       ctx.wtClipPath))) {
	     return;
	   }
	   var hAlign = align & ALIGN_HORIZONTAL_MASK;
	   var vAlign = align & ALIGN_VERTICAL_MASK;
	   var x = null, y = null;
	   switch (hAlign) {
	     case ALIGN_LEFT:
	       ctx.textAlign = 'left';
	       x = self.rect_left(rect);
	       break;
	     case ALIGN_RIGHT:
	       ctx.textAlign = 'right';
	       x = self.rect_right(rect);
	       break;
	     case ALIGN_CENTER:
	       ctx.textAlign = 'center';
	       x = self.rect_center(rect).x;
	       break;
	   }
	   switch (vAlign) {
	     case ALIGN_TOP:
	       ctx.textBaseline = 'top';
	       y = self.rect_top(rect);
	       break;
	     case ALIGN_BOTTOM:
	       ctx.textBaseline = 'bottom';
	       y = self.rect_bottom(rect);
	       break;
	     case ALIGN_MIDDLE:
	       ctx.textBaseline = 'middle';
	       y = self.rect_center(rect).y;
	       break;
	   }
	   if (x == null || y == null)
	     return;

	   var oldFillStyle = ctx.fillStyle;
	   ctx.fillStyle = ctx.strokeStyle;
	   ctx.fillText(text, x, y);
	   ctx.fillStyle = oldFillStyle;
	 };
	 this.calcYOffset = function(lineNb, nbLines, lineHeight, valign) {
           if (valign === ALIGN_MIDDLE) {
	     return - ((nbLines - 1) * lineHeight / 2.0) + lineNb * lineHeight;
	   } else if (valign === ALIGN_TOP) {
	     return lineNb * lineHeight;
	   } else if (valign === ALIGN_BOTTOM) {
	     return - (nbLines - 1 - lineNb) * lineHeight;
	   } else {
	     return 0;
	   }
	 };
	 this.drawTextOnPath = function(ctx, text, rect, transform, path, angle, lineHeight, align, softClipping) {
	   // text: array of text labels, may have newlines!
	   // path: the path that the text should be drawn along, could be transformed and stuff
	   // angle: the rotational angle the text should be drawn at, in radians
	   // lineHeight: the height of a line, duh
	   var i = 0, j = 0;
	   function x(segment) { return segment[0]; }
	   function y(segment) { return segment[1]; }
	   function type(segment) { return segment[2]; }
	   var tpath = self.transform_apply(transform, path);
	   for (i = 0; i < path.length; i++) {
	     if (i >= text.length)
	       break;
	     var seg = path[i];
	     var tseg = tpath[i];
	     var split = text[i].split("\n");
	     if (type(seg) == MOVE_TO || type(seg) == LINE_TO ||
	         type(seg) == QUAD_END || type(seg) == CUBIC_END) {
	       if (angle == 0) {
		 for (j = 0; j < split.length; j++) {
		   var yOffset = self.calcYOffset(j, split.length, lineHeight, align & ALIGN_VERTICAL_MASK);
		   self.drawText(ctx,[rect[0] + x(tseg), rect[1] + y(tseg) + yOffset, rect[2], rect[3]], align,
			 split[j], softClipping ? [x(tseg), y(tseg)] : null);
		 }
	       } else {
		 var radAngle = angle * Math.PI / 180; 
		 var r11 = Math.cos(-radAngle);
		 var r12 = -Math.sin(-radAngle);
		 var r21 = -r12;
		 var r22 = r11;
		 ctx.save();
		 ctx.transform(r11, r21, r12, r22, x(tseg), y(tseg));
		 for (j = 0; j < split.length; j++) {
		   var yOffset = self.calcYOffset(j, split.length, lineHeight, align & ALIGN_VERTICAL_MASK);
		   self.drawText(ctx,[rect[0], rect[1] + yOffset, rect[2], rect[3]], align, split[j], softClipping ? [x(tseg), y(tseg)] : null);
		 }
		 ctx.restore();
	       }
	     }
	   }
	 };
	 this.setClipPath = function(ctx, clipPath, clipPathTransform, clip) {
	   if (clip) {
	     ctx.setTransform.apply(ctx, clipPathTransform);
	     self.drawPath(ctx, clipPath, false, false, true);
	     ctx.setTransform(1, 0, 0, 1, 0, 0);
	   }

	   ctx.wtClipPath = clipPath;
	   ctx.wtClipPathTransform = clipPathTransform;
	 };
	 this.removeClipPath = function(ctx) {
	   delete ctx.wtClipPath;
	   delete ctx.wtClipPathTransform;
	 };
	 this.rect_top = function(rect) {
	    return rect[1];
	 };
	 this.rect_bottom = function(rect) {
	    return rect[1] + rect[3];
	 };
	 this.rect_right = function(rect) {
	    return rect[0] + rect[2];
	 };
	 this.rect_left = function(rect) {
	    return rect[0];
	 };
	 this.rect_topleft = function(rect) {
	    return [rect[0], rect[1]];
	 };
	 this.rect_topright = function(rect) {
	    return [rect[0] + rect[2], rect[1]];
	 };
	 this.rect_bottomleft = function(rect) {
	    return [rect[0], rect[1] + rect[3]];
	 };
	 this.rect_bottomright = function(rect) {
	    return [rect[0] + rect[2], rect[1] + rect[3]];
	 };
	 this.rect_center = function(rect) {
	    return {
	       x: (2 * rect[0] + rect[2]) / 2,
	       y: (2 * rect[1] + rect[3]) / 2
	    };
	 };
	 this.rect_normalized = function(rect) {
	    var x, y, w, h;
	    if (rect[2] > 0) {
	       x = rect[0];
	       w = rect[2];
	    } else {
	       x = rect[0] + rect[2];
	       w = - rect[2];
	    }
	    if (rect[3] > 0) {
	       y = rect[1];
	       h = rect[3];
	    } else {
	       y = rect[1] + rect[3];
	       h = - rect[3];
	    }
	    return [x,y,w,h];
	 };
      }

      return new Utils();
   })());
