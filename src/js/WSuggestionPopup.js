/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER
(1, "WSuggestionPopup",
 function(APP, el, replacerJS, matcherJS) {
   jQuery.data(el, 'obj', this);

   var self = this;
   var WT = APP.WT;

   var key_enter = 13;
   var key_tab = 9;
   var key_escape = 27;

   var key_left = 37;
   var key_up = 38;
   var key_right = 39;
   var key_down = 40;

   var selId = null, editId = null, kd = false;

   this.editKeyDown = function(edit, event) {
     var sel = selId ? WT.getElement(selId) : null;

     if (el.style.display != 'none' && sel) {
       if ((event.keyCode == key_enter) || (event.keyCode == key_tab)) {
         sel.firstChild.onclick();
         WT.cancelEvent(event);
	 setTimeout(function() { edit.focus(); }, 0);
         return false;
       } else if (event.keyCode == key_down || event.keyCode == key_up) {
         if (event.type.toUpperCase() == 'KEYDOWN') {
           kd = true;
	   WT.cancelEvent(event, WT.CancelDefaultAction);
	 }

         if (event.type.toUpperCase() == 'KEYPRESS' && kd == true) {
           WT.cancelEvent(event);
           return false;
         }

         var n = sel;
         for (n = (event.keyCode == key_down)
		  ? n.nextSibling : n.previousSibling;
              n && n.nodeName.toUpperCase() == 'DIV'
                && n.style.display == 'none';
              n = (event.keyCode == key_down)
		  ? n.nextSibling : n.previousSibling)
	   { }

         if (n && n.nodeName.toUpperCase() == 'DIV') {
           sel.className = null;
           n.className = 'sel';
           selId = n.id;
         }
         return false;
       }
     }
     return (event.keyCode != key_enter && event.keyCode != key_tab);
   };

   this.editKeyUp = function(edit, event) {
     var sel = selId ? WT.getElement(selId) : null;

     if ((event.keyCode == key_enter || event.keyCode == key_tab)
       && el.style.display == 'none')
       return;

     if (event.keyCode == key_escape
         || event.keyCode == key_left
         || event.keyCode == key_right) {
       el.style.display = 'none';
       if (event.keyCode == key_escape)
         edit.blur();
     } else {
       var text = edit.value;
       var matcher = matcherJS(edit);
       var first = null;
       var sels = el.lastChild.childNodes;
       for (var i = 0; i < sels.length; i++) {
         var child = sels[i];
         if (child.nodeName.toUpperCase() == 'DIV') {
           if (child.orig == null)
             child.orig = child.firstChild.innerHTML;
           else
             child.firstChild.innerHTML = child.orig;
           var result = matcher(child.firstChild.innerHTML);
           child.firstChild.innerHTML = result.suggestion;
           if (result.match) {
             child.style.display = 'block';
             if (first == null) first = child;
           } else
             child.style.display = 'none';
           child.className = null;
         }
       }
       if (first == null) {
         el.style.display = 'none';
       } else {
         if (el.style.display != 'block') {
           el.style.display = 'block';
           WT.positionAtWidget(el.id, edit.id,
                                       WT.Vertical);
           selId = null;
           editId = edit.id;
           sel = null;
         }
         if (!sel || (sel.style.display == 'none')) {
           selId = first.id;
           first.className = 'sel';
         } else {
           sel.className = 'sel';
         }
       }
     }
   };

   this.suggestionClicked = function(suggestion, event) {
     var edit =  WT.getElement(editId);
     var sText = suggestion.innerHTML;
     var sValue = suggestion.getAttribute('sug');
     var replacer = replacerJS;
     edit.focus();

     replacer(edit, sText, sValue);

     el.style.display = 'none';
   };

   this.delayHide = function(edit, event) {
     setTimeout(function() {
	if (el)
	 el.style.display = 'none';
       }, 300);
   };
 });