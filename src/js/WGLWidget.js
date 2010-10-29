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

   this.ctx = null;

   // Placeholders for the initializeGL and paintGL functions,
   // which will be overwritten by whatever is rendered
   this.initializeGL = function() {};
   this.paintGL = function() {};

   var dragPreviousXY = null;
   var lookAtCenter = null;
   var lookAtUpDir = null;
   var lookAtPitchRate = 0;
   var lookAtYawRate = 0;
   var cameraMatrix = null;
   var walkFrontRate = 0;
   var walkYawRate = 0;

   this.discoverContext = function() {
     if (canvas.getContext) {
       this.ctx = canvas.getContext('experimental-webgl');
       if (this.ctx == null) {
         this.ctx = canvas.getContext('webgl');
       }
       if (this.ctx == null) {
         console.log('WGLWidget: failed to get a webgl context');
       }
     }
     console.log('ctx: ' + this.ctx);
     return this.ctx;
   }

   this.setLookAtParams = function(matrix, center, up, pitchRate, yawRate) {
     cameraMatrix = matrix;
     lookAtCenter = center;
     lookAtUpDir = up;
     lookAtPitchRate = pitchRate;
     lookAtYawRate = yawRate;
   };

   this.mouseDragLookAt = function(o, event) {
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
     console.log('mouseDragLookAt after: ' + mat4.str(cameraMatrix));
     // Repaint!
     console.log('mouseDragLookAt: repaint');
     //alert('mouseDragLookAt: repaint');
     this.paintGl();
     // store mouse coord for next action
     dragPreviousXY = WT.pageCoordinates(event);
   };

   // Mouse wheel = zoom in/out
   this.mouseWheelLookAt = function(o, event) {
     //alert('foo');
     var d = WT.wheelDelta(event);
     var s = Math.pow(1.2, d);
     console.log('mouseWheelLookAt: ' + d);
     mat4.translate(cameraMatrix, lookAtCenter);
     mat4.scale(cameraMatrix, [s, s, s]);
     vec3.negate(lookAtCenter);
     mat4.translate(cameraMatrix, lookAtCenter);
     vec3.negate(lookAtCenter);
     // Repaint!
     console.log('mouseWheelLookAt: repaint');
     this.paintGl();
   };

   this.setWalkParams = function(matrix, frontRate, yawRate) {
     cameraMatrix = matrix;
     walkFrontRate = frontRate;
     walkYawRate = yawRate;
   };

   this.mouseDragWalk = function(o, event){
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
     this.paintGl();
     dragPreviousXY = WT.pageCoordinates(event);
   }


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
