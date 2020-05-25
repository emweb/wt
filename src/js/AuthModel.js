/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER
(1, JavaScriptConstructor, "AuthThrottle",
 function(WT, button, text) {
   button.wtThrottle = this;

   var timer = null, originalText = null, counter = 0;

   function restore() {
     clearInterval(timer);
     timer = null;

     WT.setHtml(button, originalText);
     button.disabled = false;
     originalText = null;
   }

   function update() {
     if (counter == 0) {
       restore();
     } else {
       WT.setHtml(button, text.replace("{1}", counter));
       --counter;
     }
   }

   this.reset = function(timeout) {
     if (timer) {
       restore();
     }

     originalText = button.innerHTML;

     counter = timeout;
     if (counter) {
       timer = setInterval(update, 1000);
       button.disabled = true;
       update();
     }
   };
 }
);
