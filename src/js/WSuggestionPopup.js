/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER
(1, "WSuggestionPopup",
 function(APP, el, replacerJS, matcherJS, filterLength, defaultValue, global) {
   $('.Wt-domRoot').add(el);

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
       filter = null, filtering = null, delayHideTimeout = null,
       lastFilterValue = null, droppedDown = false;

   this.defaultValue = defaultValue;

   /* Checks if we are (still) assisting the given edit */
   function checkEdit(edit) {
     return $(edit).hasClass("Wt-suggest-onedit")
         || $(edit).hasClass("Wt-suggest-dropdown");
   }

   function visible() {
     return el.style.display != 'none';
   }

   function hidePopup() {
     el.style.display = 'none';
   }

   function positionPopup(edit) {
     WT.positionAtWidget(el.id, edit.id, WT.Vertical, global);
   }

   function contentClicked(event) {
     var e = event || window.event;
     var line = WT.target(e);
     if (line.className == "content")
       return;

     while (line && !WT.hasTag(line, "DIV"))
       line = line.parentNode;

     if (line)
       suggestionClicked(line);
   }

   function suggestionClicked(line) {
     var suggestion = line.firstChild,
         edit = WT.getElement(editId),
         sText = suggestion.innerHTML,
         sValue = suggestion.getAttribute('sug');

     edit.focus();
     APP.emit(el, "select", line.id, edit.id);

     replacerJS(edit, sText, sValue);

     hidePopup();

     editId = null;
   };

   this.showPopup = function() {
     el.style.display = '';
     selId = null;
     lastFilterValue = null;
   };

   this.editMouseMove = function(edit, event) {
     if (!checkEdit(edit))
       return;

     var xy = WT.widgetCoordinates(edit, event);
     if (xy.x > edit.offsetWidth - 16)
       edit.style.cursor = 'default';
     else
       edit.style.cursor = '';
   };

   this.editClick = function(edit, event) {
     if (!checkEdit(edit))
       return;

     var xy = WT.widgetCoordinates(edit, event);
     if (xy.x > edit.offsetWidth - 16) {
       if (editId != edit.id || !visible()) {
	 hidePopup();
	 editId = edit.id;
	 droppedDown = true;
	 self.refilter();
       } else {
	 editId = null;
	 hidePopup();
       }
     }
   };

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
     if (!checkEdit(edit))
       return true;

     if (editId != edit.id) {
       if ($(edit).hasClass("Wt-suggest-onedit")) {
	 editId = edit.id;
	 droppedDown = false;
       } else if ($(edit).hasClass("Wt-suggest-dropdown")
		  && event.keyCode == key_down) {
	 editId = edit.id;
	 droppedDown = true;
       } else {
	 editId = null;
	 return true;
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
           sel.className = '';
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

   function scrollToSelected(sel) {
     var p = sel.parentNode;

     if (sel.offsetTop + sel.offsetHeight > p.scrollTop + p.clientHeight)
       p.scrollTop = sel.offsetTop + sel.offsetHeight - p.clientHeight;
     else if (sel.offsetTop < p.scrollTop)
       p.scrollTop = sel.offsetTop;
   }

   /*
    * Refilter the current selection list based on the edit value.
    */
   this.refilter = function() {
     var sel = selId ? WT.getElement(selId) : null,
         edit = WT.getElement(editId),
         matcher = matcherJS(edit),
         sels = el.lastChild.childNodes,
         text = matcher(null);

     lastFilterValue = edit.value;

     if (filterLength) {
       if (text.length < filterLength && !droppedDown) {
	 hidePopup();
	 return;
       } else {
	 var nf = text.substring(0, filterLength);
	 if (nf != filter) {
	   if (nf != filtering) {
	     filtering = nf;
	     APP.emit(el, "filter", nf);
	   }

	   if (!droppedDown) {
	     // better would be to set a Loading indicator
	     hidePopup();
	     return;
	   }
	 }
       }
     }

     var first = null, toselect = null,
         showall = droppedDown && text.length == 0,
         i, il;

     for (i = 0, il = sels.length; i < il; ++i) {
       var child = sels[i];
       if (WT.hasTag(child, 'DIV')) {
         if (child.orig == null) {
           child.orig = child.firstChild.innerHTML;
	 }

	 var result = matcher(child.orig),
	     match = showall || result.match;

	 if (result.suggestion != child.firstChild.innerHTML)
	   child.firstChild.innerHTML = result.suggestion;

         if (match) {
	   if (child.style.display != '')
             child.style.display = '';
           if (first == null)
	     first = child;
	   if (i == this.defaultValue) {
	     toselect = child;
	   }
         } else if (child.style.display != 'none')
           child.style.display = 'none';

	 if (child.className != '')
	   child.className = '';
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
	 sel = toselect || first ;
	 sel.parentNode.scrollTop = 0;
         selId = sel.id;
       }

       sel.className = 'sel';
       scrollToSelected(sel);
     }
   };

   this.editKeyUp = function(edit, event) {
     if (editId == null)
       return;

     if (!checkEdit(edit))
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
	 if ($(edit).hasClass("Wt-suggest-dropdown"))
	   hidePopup();
	 else
	   edit.blur();
       }
     } else {
       if (edit.value != lastFilterValue)
	 self.refilter();
       else {
	 var sel = selId ? WT.getElement(selId) : null;
	 if (sel)
	   scrollToSelected(sel);
       }
     }
   };

   el.lastChild.onclick = contentClicked;

   /*
    * In Safari, scrolling causes the edit to lose focus, but we don't want
    * that. Can it be avoided? In any case, this fixes it.
    */
   el.lastChild.onscroll = function() {
     if (delayHideTimeout) {
       clearTimeout(delayHideTimeout);
       var edit = WT.getElement(editId);
       if (edit)
	 edit.focus();
     }
   };

   this.delayHide = function(edit, event) {
     delayHideTimeout = setTimeout(function() {
       delayHideTimeout = null;
       if (el && (edit == null || editId == edit.id))
	   hidePopup();
       }, 300);
   };
 });

WT_DECLARE_WT_MEMBER
(2, "WSuggestionPopupStdMatcher",
 function(highlightBeginTag, highlightEndTag, listSeparator, whiteSpace,
	  wordSeparators, appendReplacedText) {
   function parseEdit(edit) {
     var value = edit.value;
     var pos = edit.selectionStart ? edit.selectionStart : value.length;

     var start = listSeparator
       ? value.lastIndexOf(listSeparator, pos - 1) + 1 : 0;

     while ((start < pos)
            && (whiteSpace.indexOf(value.charAt(start)) != -1))
       ++start;

     return { start: start, end: pos };
   }

   this.match = function(edit) {
     var range = parseEdit(edit);
     var value = edit.value.substring(range.start, range.end);

     var regexp = "^";
     if (wordSeparators.length != 0)
       regexp = "(^|(?:[" + wordSeparators + "]))";

     regexp += "(" + value.replace
       (new RegExp("([\\^\\\\\\][\\-.$*+?()|{}])","g"), "\\$1") + ")";

     regexp = new RegExp(regexp, "gi");

     return function(suggestion) {
       if (!suggestion)
	 return value;

       var matched = false;

       if (value.length) {
	 var highlighted
	   = suggestion.replace(regexp, "$1" + highlightBeginTag + "$2"
				+ highlightEndTag);
	 if (highlighted != suggestion) {
	   matched = true;
	   suggestion = highlighted;
	 }
       }

       return { match: matched, suggestion: suggestion };
     };
   };

   this.replace = function(edit, suggestionText, suggestionValue) {
     var range = parseEdit(edit);

     var nv = edit.value.substring(0, range.start) + suggestionValue
       + appendReplacedText;

     if (range.end < edit.value.length)
      nv += edit.value.substring(range.end, edit.value.length);

     edit.value = nv;

     if (edit.selectionStart) {
       edit.selectionStart = range.start + suggestionValue.length
	 + appendReplacedText.length;
       edit.selectionEnd = edit.selectionStart;
     }
   };
 });
