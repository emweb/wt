/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER
(1, "WSuggestionPopup",
 function(APP, el, replacerJS, matcherJS, filterLength) {
   document.body.appendChild(el);

   jQuery.data(el, 'obj', this);

   var self = this;
   var WT = APP.WT;

   var key_tab = 9;
   var key_enter = 13;
   var key_escape = 27;

   var key_pup = 33;
   var key_pdown = 34;
   var key_left = 37;
   var key_up = 38;
   var key_right = 39;
   var key_down = 40;

   var selId = null, editId = null, kd = false,
       filter = null, filtering = null;

   function visible() {
     return el.style.display != 'none';
   }

   function hidePopup() {
     el.style.display = 'none';
   }

   function positionPopup(edit) {
     WT.positionAtWidget(el.id, edit.id, WT.Vertical);
   }

   function contentClicked(event) {
     var e = event||window.event;
     var line = e.target || e.srcElement;
     if (line.className == "content")
       return;

     if (!WT.hasTag(line, "DIV"))
       line = line.parentNode;

     suggestionClicked(line);
   }

   function suggestionClicked(line) {
     var suggestion = line.firstChild,
         edit = WT.getElement(editId),
         sText = suggestion.innerHTML,
         sValue = suggestion.getAttribute('sug');

     edit.focus();

     replacerJS(edit, sText, sValue);

     hidePopup();

     editId = null;
   };

   this.showPopup = function() {
     el.style.display = '';
     selId = null;
   };

   this.editMouseMove = function(edit, event) {
     var xy = WT.widgetCoordinates(edit, event);
     if (xy.x > edit.offsetWidth - 16)
       edit.style.cursor = 'default';
     else
       edit.style.cursor = '';
   }

   this.editClick = function(edit, event) {
     if (event.clientX > edit.offsetWidth - 16) {
       editId = edit.id;
       self.refilter();
     }
   }

   function next(n, down) {
     for (n = down ? n.nextSibling : n.previousSibling;
	  n;
	  n = down ? n.nextSibling : n.previousSibling) {
       if (WT.hasTag(n, 'DIV'))
	 if (n.style.display != 'none')
	   return n;
     }

     return null;
   }

   this.editKeyDown = function(edit, event) {
     if (editId != edit.id) {
       if ($(edit).hasClass("Wt-suggest-onedit"))
	 editId = edit.id;
       else if ($(edit).hasClass("Wt-suggest-dropdown")
		&& event.keyCode == key_down)
	 editId = edit.id;
       else {
	 editId = null;
	 return;
       }
     }

     var sel = selId ? WT.getElement(selId) : null;

     if (visible() && sel) {
       if ((event.keyCode == key_enter) || (event.keyCode == key_tab)) {
	 /*
	  * Select currently selectd
	  */
         suggestionClicked(sel);
         WT.cancelEvent(event);
	 setTimeout(function() { edit.focus(); }, 0);
         return false;
       } else if (   event.keyCode == key_down
		  || event.keyCode == key_up
		  || event.keyCode == key_pdown
		  || event.keyCode == key_pup) {
	 /*
	  * Handle navigation in list
	  */
         if (event.type.toUpperCase() == 'KEYDOWN') {
           kd = true;
	   WT.cancelEvent(event, WT.CancelDefaultAction);
	 }

         if (event.type.toUpperCase() == 'KEYPRESS' && kd == true) {
           WT.cancelEvent(event);
           return false;
         }

	 /*
	  * Find next selected node
	  */
         var n = sel,
	     down = event.keyCode == key_down || event.keyCode == key_pdown,
	     count = (event.keyCode == key_pdown || event.keyCode == key_pup ?
		      el.clientHeight / sel.offsetHeight : 1),
	     i;

	 for (i = 0; n && i < count; ++i) {
	   var l = next(n, down);
	   if (!l)
	     break;
	   n = l;
	 }

         if (n && WT.hasTag(n, 'DIV')) {
           sel.className = null;
           n.className = 'sel';
           selId = n.id;
         }

         return false;
       }
     }
     return (event.keyCode != key_enter && event.keyCode != key_tab);
   };

   this.filtered = function(f) {
     filter = f;
     self.refilter();
   };

   /*
    * Refilter the current selection list based on the edit value.
    */
   this.refilter = function() {
     var sel = selId ? WT.getElement(selId) : null,
         edit = WT.getElement(editId),
         matcher = matcherJS(edit),
         canhide = !$(edit).hasClass("Wt-suggest-dropdown"),
         sels = el.lastChild.childNodes,
         text = matcher(null);

     if (filterLength) {
       if (canhide && text.length < filterLength) {
	 hidePopup();
	 return;
       } else {
	 var nf = text.substring(0, filterLength);
	 if (nf != filter) {
	   if (nf != filtering) {
	     filtering = nf;
	     APP.emit(el, "filter", nf);
	   }
	   if (canhide) {
	     hidePopup();
	     return;
	   }
	 }
       }
     }

     var first = null,
         sels = el.lastChild.childNodes,
         showall = !canhide && text.length == 0;

     for (var i = 0; i < sels.length; i++) {
       var child = sels[i];
       if (WT.hasTag(child, 'DIV')) {
         if (child.orig == null)
           child.orig = child.firstChild.innerHTML;
         else
           child.firstChild.innerHTML = child.orig;

         var match = showall;

	 if (!showall) {
           var result = matcher(child.firstChild.innerHTML);
           child.firstChild.innerHTML = result.suggestion;
           match = result.match;
         }

         if (match) {
           child.style.display = '';
           if (first == null)
	     first = child;
         } else
           child.style.display = 'none';

         child.className = null;
       }
     }

     if (first == null) {
       hidePopup();
     } else {
       if (!visible()) {
	 positionPopup(edit);
	 self.showPopup();
	 sel = null;
       }

       if (!sel || (sel.style.display == 'none')) {
         selId = first.id;
	 sel = first;
	 sel.parentNode.scrollTop = 0;
       }

       /*
	* Make sure currently selected is scrolled into view
	*/
       sel.className = 'sel';
       var p = sel.parentNode;
       if (sel.offsetTop + sel.offsetHeight > p.scrollTop + p.clientHeight)
	 sel.scrollIntoView(false);
       else if (sel.offsetTop < p.scrollTop)
         sel.scrollIntoView(true);
     }
   };

   this.editKeyUp = function(edit, event) {
     if (editId == null)
       return;

     if ((event.keyCode == key_enter || event.keyCode == key_tab)
       && el.style.display == 'none')
       return;

     if (event.keyCode == key_escape
         || event.keyCode == key_left
         || event.keyCode == key_right) {
       el.style.display = 'none';
       if (event.keyCode == key_escape) {
	 editId = null;
         edit.blur();
       }
     } else {
       self.refilter();
     }
   };

   el.lastChild.onclick = contentClicked;

   this.delayHide = function(edit, event) {
     setTimeout(function() {
	if (el)
	  hidePopup();
       }, 300);
   };
 });
