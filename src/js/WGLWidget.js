/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER
(1, "WGLWidget",
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

   var dragPreviousXY = null;
   var lookAtCenter = null;
   var lookAtUpDir = null;
   var lookAtPitchRate = 0;
   var lookAtYawRate = 0;
   var cameraMatrix = null;
   var walkFrontRate = 0;
   var walkYawRate = 0;

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
         parentNode.replaceChild(alternative, canvas);
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

   this.mouseDragLookAt = function(o, event) {
     if (this.ctx == null) return; // no WebGL support
     var c = WT.pageCoordinates(event);
     var dx=(c.x - dragPreviousXY.x);
     var dy=(c.y - dragPreviousXY.y);
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
     //console.log('mouseDragLookAt after: ' + mat4.str(cameraMatrix));
     // Repaint!
     //console.log('mouseDragLookAt: repaint');
     this.paintGL();
     // store mouse coord for next action
     dragPreviousXY = WT.pageCoordinates(event);
   };

   // Mouse wheel = zoom in/out
   this.mouseWheelLookAt = function(o, event) {
     if (this.ctx == null) return; // no WebGL support
     WT.cancelEvent(event);
     //alert('foo');
     var d = WT.wheelDelta(event);
     var s = Math.pow(1.2, d);
     mat4.translate(cameraMatrix, lookAtCenter);
     mat4.scale(cameraMatrix, [s, s, s]);
     vec3.negate(lookAtCenter);
     mat4.translate(cameraMatrix, lookAtCenter);
     vec3.negate(lookAtCenter);
     // Repaint!
     this.paintGL();
   };

   this.setWalkParams = function(matrix, frontRate, yawRate) {
     cameraMatrix = matrix;
     walkFrontRate = frontRate;
     walkYawRate = yawRate;
   };

   this.mouseDragWalk = function(o, event){
     if (this.ctx == null) return; // no WebGL support
     var c = WT.pageCoordinates(event);
     var dx=(c.x - dragPreviousXY.x);
     var dy=(c.y - dragPreviousXY.y);
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


   this.mouseDown = function(o, event) {
     WT.capture(null);
     WT.capture(canvas);

     dragPreviousXY = WT.pageCoordinates(event);
   };

   this.mouseUp = function(o, event) {
     if (dragPreviousXY != null)
       dragPreviousXY = null;
   };

 });
