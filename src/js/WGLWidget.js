/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER
(1, JavaScriptConstructor, "WGLWidget",
 function(APP, canvas) {
   jQuery.data(canvas, 'obj', this);

   var self = this;
   var WT = APP.WT;

   var vec3 = WT.glMatrix.vec3;
   var mat3 = WT.glMatrix.mat3;
   var mat4 = WT.glMatrix.mat4;

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
   this.jsMatrices = new Array();

   var dragPreviousXY = null;
   var lookAtCenter = null;
   var lookAtUpDir = null;
   var lookAtPitchRate = 0;
   var lookAtYawRate = 0;
   var cameraMatrix = null;
   var walkFrontRate = 0;
   var walkYawRate = 0;
   var pinchWidth = null;
   var singleTouch = null;
   var doubleTouch = null;

   this.discoverContext = function(noGLHandler) {
     if (canvas.getContext) {
       try {
         this.ctx = canvas.getContext('webgl', {antialias: true});
       } catch (e) {}
       if (this.ctx == null) {
         try {
           this.ctx = canvas.getContext('experimental-webgl');
         } catch (e) {}
       }
       if (this.ctx == null) {
         var alternative = canvas.firstChild;
         var parentNode = canvas.parentNode;
         parentNode.insertBefore(alternative, canvas);
         canvas.style.display = 'none';
         noGLHandler();
       }
     }
     return this.ctx;
   };

   this.setLookAtParams = function(matrix, center, up, pitchRate, yawRate) {
     cameraMatrix = matrix;
     lookAtCenter = center;
     lookAtUpDir = up;
     lookAtPitchRate = pitchRate;
     lookAtYawRate = yawRate;
   };

   this.rotate = function(newCoords) {
     var dx=(newCoords.x - dragPreviousXY.x);
     var dy=(newCoords.y - dragPreviousXY.y);
     var s=vec3.create();
     s[0]=cameraMatrix[0];
     s[1]=cameraMatrix[4];
     s[2]=cameraMatrix[8];
     var r=mat4.create();
     mat4.identity(r);
     mat4.translate(r, lookAtCenter);
     mat4.rotate(r, dy * lookAtPitchRate, s);
     mat4.rotate(r, dx * lookAtYawRate, lookAtUpDir);
     vec3.negate(lookAtCenter);
     mat4.translate(r, lookAtCenter);
     vec3.negate(lookAtCenter);
     mat4.multiply(cameraMatrix,r,cameraMatrix);
     //console.log('rotate after: ' + mat4.str(cameraMatrix));
     // Repaint!
     //console.log('rotate: repaint');
     this.paintGL();
     // store mouse coord for next action
     dragPreviousXY = newCoords;
   };

   this.zoom = function(delta) {
     var s = Math.pow(1.2, delta);
     mat4.translate(cameraMatrix, lookAtCenter);
     mat4.scale(cameraMatrix, [s, s, s]);
     vec3.negate(lookAtCenter);
     mat4.translate(cameraMatrix, lookAtCenter);
     vec3.negate(lookAtCenter);
     // Repaint!
     this.paintGL();
   }

   // Mouse wheel = zoom in/out
   this.mouseWheelLookAt = function(o, event) {
     WT.cancelEvent(event);
     //alert('foo');
     var d = WT.wheelDelta(event);
     this.zoom(d);
   };

   this.setWalkParams = function(matrix, frontRate, yawRate) {
     cameraMatrix = matrix;
     walkFrontRate = frontRate;
     walkYawRate = yawRate;
   };

   this.walk = function(newCoords){
     var dx=(newCoords.x - dragPreviousXY.x);
     var dy=(newCoords.y - dragPreviousXY.y);
     var r=mat4.create();
     mat4.identity(r);
     mat4.rotateY(r, dx * walkYawRate);
     var t=vec3.create();
     t[0]=0;
     t[1]=0;
     t[2]=-walkFrontRate * dy;
     mat4.translate(r, t);
     mat4.multiply(r, cameraMatrix, cameraMatrix);
     this.paintGL();
     dragPreviousXY = WT.pageCoordinates(event);
   };

   this.mouseDragWalk = function(o, event) {
     if (dragPreviousXY == null)
	 return;
     var c = WT.pageCoordinates(event);
     this.walk(c);
   };
   this.mouseDragLookAt = function(o, event) {
     if (dragPreviousXY == null)
       return;
     var c = WT.pageCoordinates(event);
     this.rotate(c);
   };

   this.mouseDown = function(o, event) {
     if (event.button != 0 && !WT.isIElt9)
	 return;
     else if (WT.isIElt9 && event.button != 1)
	 return;

     WT.capture(null);
     WT.capture(canvas);

     dragPreviousXY = WT.pageCoordinates(event);
   };

   this.mouseUp = function(o, event) {
     if (dragPreviousXY != null)
       dragPreviousXY = null;
   };

   this.touchStart = function(o, event) {
     singleTouch = event.touches.length == 1 ? true : false;
     doubleTouch = event.touches.length == 2 ? true : false;

     if (singleTouch) {
       WT.capture(null);
       WT.capture(canvas);
       dragPreviousXY = WT.pageCoordinates(event.touches[0]);
     } else if (doubleTouch) {
       var c0 = WT.pageCoordinates(event.touches[0]);
       var c1 = WT.pageCoordinates(event.touches[1]);
       pinchWidth = Math.sqrt( (c0.x-c1.x)*(c0.x-c1.x) + (c0.y-c1.y)*(c0.y-c1.y) );
     } else {
       return;
     }
     event.preventDefault();
   };
   this.touchEnd = function(o, event) {
     var noTouch = event.touches.length == 0 ? true : false;
     singleTouch = event.touches.length == 1 ? true : false;
     doubleTouch = event.touches.length == 2 ? true : false;

     if (noTouch)
       this.mouseUp(null, null);
     if (singleTouch || doubleTouch)
       this.touchStart(o, event);
   };
   this.touchMoved = function(o, event) {
     if ( (!singleTouch) && (!doubleTouch) )
       return;

     event.preventDefault();
     if (singleTouch)
       this.mouseDragLookAt(o, event.touches[0]);
     if (doubleTouch) {
       var c0 = WT.pageCoordinates(event.touches[0]);
       var c1 = WT.pageCoordinates(event.touches[1]);
       var d = Math.sqrt( (c0.x-c1.x)*(c0.x-c1.x) + (c0.y-c1.y)*(c0.y-c1.y) );
       var scale = d / pinchWidth;
       if (Math.abs(scale-1) < 0.05) {
	 return;
       } else if (scale > 1) {
	 scale = 1;
       } else {
	 scale = -1;
       }
       pinchWidth = d;
       this.zoom(scale);
     }
   };

   // To be called after a load of buffer/texture completes; will
   // check if it is safe to render
   this.handlePreload = function() {
     if (this.preloadingTextures == 0 && this.preloadingBuffers == 0) {
       if(this.initialized){
         var key;
         // execute all updates scheduled in o.updates
         for(key in this.updates) this.updates[key]();
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

   function encodeJSMatrices() {
     var obj = jQuery.data(canvas, 'obj');
     var str = '';
     for (var index=0; index < obj.jsMatrices.length; index++) {
       str += index + ':';
       for (var i=0; i < obj.jsMatrices[index].length; i++) {
	 str += obj.jsMatrices[index][i];
	 if (i != 15) {
	   str += ',';
	 } else {
	   str += ';';
	 }
       }
     }
     return str;
   }

   canvas.wtEncodeValue = encodeJSMatrices;
 });
