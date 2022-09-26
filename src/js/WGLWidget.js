/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER(1, JavaScriptConstructor, "WGLWidget", function(APP, canvas) {
  canvas.wtObj = this;

  const self = this;
  const WT = APP.WT;

  const vec3 = WT.glMatrix.vec3;
  const mat4 = WT.glMatrix.mat4;

  this.ctx = null;

  // Placeholders for the initializeGL and paintGL functions,
  // which will be overwritten by whatever is rendered
  this.initializeGL = function() {};
  this.paintGL = function() {};
  this.resizeGL = function() {};
  this.updates = new Array();
  this.initialized = false;
  this.preloadingTextures = 0;
  this.preloadingBuffers = 0;
  this.jsValues = {};

  this.discoverContext = function(noGLHandler, antialiasingEnabled) {
    if (canvas.getContext) {
      try {
        this.ctx = canvas.getContext("webgl", { antialias: antialiasingEnabled });
      } catch (e) {
        // empty
      }
      if (this.ctx === null) {
        try {
          this.ctx = canvas.getContext("experimental-webgl", { antialias: antialiasingEnabled });
        } catch (e) {
          // empty
        }
      }
      if (this.ctx === null) {
        const alternative = canvas.firstChild;
        const parentNode = canvas.parentNode;
        parentNode.insertBefore(alternative, canvas);
        canvas.style.display = "none";
        noGLHandler();
      }
    }
    return this.ctx;
  };

  if (canvas.addEventListener) {
    canvas.addEventListener("webglcontextlost", function(event) {
      event.preventDefault();
      self.initialized = false;
    }, false);
    canvas.addEventListener("webglcontextrestored", function(_event) {
      APP.emit(canvas, "contextRestored");
    }, false);
  }

  let mouseHandler = null;

  this.setMouseHandler = function(newMouseHandler) {
    mouseHandler = newMouseHandler;
    if (mouseHandler.setTarget) {
      mouseHandler.setTarget(this);
    }
  };

  this.LookAtMouseHandler = function(matrix, center, up, pitchRate, yawRate) {
    const cameraMatrix = matrix;
    const lookAtCenter = center;
    const lookAtUpDir = up;
    const lookAtPitchRate = pitchRate;
    const lookAtYawRate = yawRate;
    let pinchWidth = null;
    let singleTouch = null;
    let doubleTouch = null;
    let dragPreviousXY = null;

    const thisHandler = this;

    this.mouseDown = function(o, event) {
      WT.capture(null);
      WT.capture(canvas);

      dragPreviousXY = WT.pageCoordinates(event);
    };

    this.mouseUp = function(_o, _event) {
      if (dragPreviousXY !== null) {
        dragPreviousXY = null;
      }
    };

    this.mouseDrag = function(o, event) {
      if (dragPreviousXY === null) {
        return;
      }
      const c = WT.pageCoordinates(event);
      if (WT.buttons === 1) {
        rotate(c);
      }
    };

    // Mouse wheel = zoom in/out
    this.mouseWheel = function(o, event) {
      WT.cancelEvent(event);
      const d = WT.wheelDelta(event);
      zoom(d);
    };

    function zoom(delta) {
      const s = Math.pow(1.2, delta);
      mat4.translate(cameraMatrix, lookAtCenter);
      mat4.scale(cameraMatrix, [s, s, s]);
      vec3.negate(lookAtCenter);
      mat4.translate(cameraMatrix, lookAtCenter);
      vec3.negate(lookAtCenter);
      // Repaint!
      self.paintGL();
    }

    function rotate(newCoords) {
      const prevPitchCos = cameraMatrix[5] / vec3.length([cameraMatrix[1], cameraMatrix[5], cameraMatrix[9]]);
      const prevPitchSin = cameraMatrix[6] / vec3.length([cameraMatrix[2], cameraMatrix[6], cameraMatrix[10]]);
      const prevPitch = Math.atan2(prevPitchSin, prevPitchCos);
      const dx = (newCoords.x - dragPreviousXY.x);
      const dy = (newCoords.y - dragPreviousXY.y);
      const s = vec3.create();
      s[0] = cameraMatrix[0];
      s[1] = cameraMatrix[4];
      s[2] = cameraMatrix[8];
      const r = mat4.create();
      mat4.identity(r);
      mat4.translate(r, lookAtCenter);
      let dPitch = dy * lookAtPitchRate;
      if (Math.abs(prevPitch + dPitch) >= Math.PI / 2) {
        const sign = prevPitch > 0 ? 1 : -1;
        dPitch = sign * Math.PI / 2 - prevPitch;
      }
      mat4.rotate(r, dPitch, s);
      mat4.rotate(r, dx * lookAtYawRate, lookAtUpDir);
      vec3.negate(lookAtCenter);
      mat4.translate(r, lookAtCenter);
      vec3.negate(lookAtCenter);
      mat4.multiply(cameraMatrix, r, cameraMatrix);
      // Repaint!
      self.paintGL();
      // store mouse coord for next action
      dragPreviousXY = newCoords;
    }

    this.touchStart = function(o, event) {
      singleTouch = event.touches.length === 1 ? true : false;
      doubleTouch = event.touches.length === 2 ? true : false;

      if (singleTouch) {
        WT.capture(null);
        WT.capture(canvas);
        dragPreviousXY = WT.pageCoordinates(event.touches[0]);
      } else if (doubleTouch) {
        const c0 = WT.pageCoordinates(event.touches[0]);
        const c1 = WT.pageCoordinates(event.touches[1]);
        pinchWidth = Math.sqrt((c0.x - c1.x) * (c0.x - c1.x) + (c0.y - c1.y) * (c0.y - c1.y));
      } else {
        return;
      }
      event.preventDefault();
    };

    this.touchEnd = function(o, event) {
      const noTouch = event.touches.length === 0 ? true : false;
      singleTouch = event.touches.length === 1 ? true : false;
      doubleTouch = event.touches.length === 2 ? true : false;

      if (noTouch) {
        thisHandler.mouseUp(null, null);
      }
      if (singleTouch || doubleTouch) {
        thisHandler.touchStart(o, event);
      }
    };

    this.touchMoved = function(o, event) {
      if ((!singleTouch) && (!doubleTouch)) {
        return;
      }

      event.preventDefault();
      if (singleTouch) {
        if (dragPreviousXY === null) {
          return;
        }
        const c = WT.pageCoordinates(event);
        rotate(c);
      }
      if (doubleTouch) {
        const c0 = WT.pageCoordinates(event.touches[0]);
        const c1 = WT.pageCoordinates(event.touches[1]);
        const d = Math.sqrt((c0.x - c1.x) * (c0.x - c1.x) + (c0.y - c1.y) * (c0.y - c1.y));
        let scale = d / pinchWidth;
        if (Math.abs(scale - 1) < 0.05) {
          return;
        } else if (scale > 1) {
          scale = 1;
        } else {
          scale = -1;
        }
        pinchWidth = d;
        zoom(scale);
      }
    };
  };

  this.WalkMouseHandler = function(matrix, frontRate, yawRate) {
    const cameraMatrix = matrix;
    const walkFrontRate = frontRate;
    const walkYawRate = yawRate;
    let dragPreviousXY = null;

    this.mouseDown = function(o, event) {
      WT.capture(null);
      WT.capture(canvas);

      dragPreviousXY = WT.pageCoordinates(event);
    };

    this.mouseUp = function(_o, _event) {
      if (dragPreviousXY !== null) {
        dragPreviousXY = null;
      }
    };

    this.mouseDrag = function(_o, event) {
      if (dragPreviousXY === null) {
        return;
      }
      const c = WT.pageCoordinates(event);
      walk(c);
    };

    function walk(newCoords) {
      const dx = (newCoords.x - dragPreviousXY.x);
      const dy = (newCoords.y - dragPreviousXY.y);
      const r = mat4.create();
      mat4.identity(r);
      mat4.rotateY(r, dx * walkYawRate);
      const t = vec3.create();
      t[0] = 0;
      t[1] = 0;
      t[2] = -walkFrontRate * dy;
      mat4.translate(r, t);
      mat4.multiply(r, cameraMatrix, cameraMatrix);
      self.paintGL();

      dragPreviousXY = WT.pageCoordinates(event);
    }
  };

  this.mouseDrag = function(o, event) {
    if ((this.initialized || !this.ctx) && mouseHandler && mouseHandler.mouseDrag) {
      mouseHandler.mouseDrag(o, event);
    }
  };
  this.mouseMove = function(o, event) {
    if ((this.initialized || !this.ctx) && mouseHandler && mouseHandler.mouseMove) {
      mouseHandler.mouseMove(o, event);
    }
  };
  this.mouseDown = function(o, event) {
    if ((this.initialized || !this.ctx) && mouseHandler && mouseHandler.mouseDown) {
      mouseHandler.mouseDown(o, event);
    }
  };
  this.mouseUp = function(o, event) {
    if ((this.initialized || !this.ctx) && mouseHandler && mouseHandler.mouseUp) {
      mouseHandler.mouseUp(o, event);
    }
  };
  this.mouseWheel = function(o, event) {
    if ((this.initialized || !this.ctx) && mouseHandler && mouseHandler.mouseWheel) {
      mouseHandler.mouseWheel(o, event);
    }
  };
  this.touchStart = function(o, event) {
    if ((this.initialized || !this.ctx) && mouseHandler && mouseHandler.touchStart) {
      mouseHandler.touchStart(o, event);
    }
  };
  this.touchEnd = function(o, event) {
    if ((this.initialized || !this.ctx) && mouseHandler && mouseHandler.touchEnd) {
      mouseHandler.touchEnd(o, event);
    }
  };
  this.touchMoved = function(o, event) {
    if ((this.initialized || !this.ctx) && mouseHandler && mouseHandler.touchMoved) {
      mouseHandler.touchMoved(o, event);
    }
  };

  // To be called after a load of buffer/texture completes; will
  // check if it is safe to render
  this.handlePreload = function() {
    if (this.preloadingTextures === 0 && this.preloadingBuffers === 0) {
      if (this.initialized) {
        // execute all updates scheduled in o.updates
        for (const update of this.updates) {
          update();
        }
        this.updates = new Array();
        // Delay calling of resizeGL() to after updates are executed
        this.resizeGL();
        this.paintGL();
      } else {
        // initializeGL will call updates and resizeGL
        this.initializeGL();
        this.resizeGL();
        this.paintGL();
      }
    } else {
      // still waiting for data to arrive...
    }
  };

  function encodeJSValues() {
    const obj = canvas.wtObj;
    let str = "";
    for (const [index, value] of Object.entries(obj.jsValues)) {
      str += index + ":";
      for (let i = 0; i < value.length; i++) {
        str += value[i];
        if (i !== value.length - 1) {
          str += ",";
        } else {
          str += ";";
        }
      }
    }
    return str;
  }
  canvas.wtEncodeValue = encodeJSValues;

  // For server-side rendering: avoid that image loads are aborted because
  // URL is changed too quickly (before image is fully loaded)
  let nextImage = null;
  const loader = new Image();
  loader.busy = false;
  loader.onload = function() {
    canvas.src = loader.src;
    if (nextImage !== null) {
      loader.src = nextImage;
    } else {
      loader.busy = false;
    }
    nextImage = null;
  };
  loader.onerror = loader.onload;

  this.loadImage = function(url) {
    if (loader.busy) {
      nextImage = url;
    } else {
      loader.src = url;
      loader.busy = true;
    }
  };
});
