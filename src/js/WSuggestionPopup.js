/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER
(1, JavaScriptConstructor, "WSuggestionPopup",
 function(APP, el, replacerJS, matcherJS, filterMinLength, filterMore,
	  defaultValue, isDropDownIconUnfiltered) {
   $('.Wt-domRoot').add(el);

   el.wtObj = this;

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
     filter = null, filtering = null, filterPartial = filterMore,
     delayHideTimeout = null,
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

   function positionPopup(edit) {
     el.style.display='block';
     WT.positionAtWidget(el.id, edit.id, WT.Vertical);
   }

   function contentClicked(event) {
     var e = event || window.event;
     var line = WT.target(e);
     if (WT.hasTag(line, "UL"))
       return;

     while (line && !WT.hasTag(line, "LI"))
       line = line.parentNode;

     if (line)
       suggestionClicked(line);
   }

   function suggestionClicked(line) {
     var suggestion = line.firstChild.firstChild,
         edit = WT.getElement(editId),
         sText = suggestion.innerHTML,
         sValue = suggestion.getAttribute('sug');

     edit.focus();
     replacerJS(edit, sText, sValue);

     APP.emit(el, "select", line.id, edit.id);

     hidePopup();

     editId = null;
   };

   var keyDownFun = null;

   this.showPopup = function(edit) {
     el.style.display = 'block';
     selId = null;
     lastFilterValue = null;
     keyDownFun = edit.onkeydown;
     edit.onkeydown = function(event) {
       var e=event||window.event,o=this;
       self.editKeyDown(o, e);
     };
   };

   function hidePopup() {
     el.style.display = 'none';
     if (editId != null && keyDownFun != null) {
       var edit = WT.getElement(editId);
       edit.onkeydown = keyDownFun;
       keyDownFun = null;
     }
   }

   this.editMouseMove = function(edit, event) {
     if (!checkEdit(edit))
       return;

     var xy = WT.widgetCoordinates(edit, event);
     if (xy.x > edit.offsetWidth - 16)
       edit.style.cursor = 'default';
     else
       edit.style.cursor = '';
   };

   this.showAt = function(edit, value) {
     hidePopup();
     editId = edit.id;
     droppedDown = true;
     self.refilter(value);
   };

   this.editClick = function(edit, event) {
     if (!checkEdit(edit))
       return;

     var xy = WT.widgetCoordinates(edit, event);
     if (xy.x > edit.offsetWidth - 16) {
       if (editId != edit.id || !visible()) {
	 self.showAt(edit, '');
       } else {
	 hidePopup();
	 editId = null;
       }
     }
   };

   function next(n, down) {
     for (n = down ? n.nextSibling : n.previousSibling;
	  n;
	  n = down ? n.nextSibling : n.previousSibling) {
       if (WT.hasTag(n, 'LI'))
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

         if (n && WT.hasTag(n, 'LI')) {
           sel.className = '';
           n.className = 'active';
           selId = n.id;
         }

         return false;
       }
     }
     return (event.keyCode != key_enter && event.keyCode != key_tab);
   };

   this.filtered = function(f, partial) {
     filter = f;
     filterPartial = partial;
     self.refilter(lastFilterValue);
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
   this.refilter = function(value) {
	 if(!editId) {
	   //If edit is null we probably have already choosen a suggestion and 
	   //we therefore don't need to refilter!
	   return;
	 }

     var sel = selId ? WT.getElement(selId) : null,
         edit = WT.getElement(editId),
         matcher = matcherJS(edit),
         sels = el.childNodes,
         text = (isDropDownIconUnfiltered && value != null)  ? value : matcher(null);

     lastFilterValue = isDropDownIconUnfiltered ? value : edit.value;

     if (filterMinLength > 0 || filterMore) {
       if (text.length < filterMinLength && !droppedDown) {
	 hidePopup();
	 return;
       } else {
	 var nf = filterPartial ?
	   text :
	   text.substring(0, Math.max(filter !== null ? filter.length : 0,
				      filterMinLength));

	 if (nf != filter) {
	   if (nf != filtering) {
	     filtering = nf;
	     APP.emit(el, "filter", nf);
	   }
	 }
       }
     }

     var first = null, toselect = null,
         showall = droppedDown && text.length == 0,
         i, il;

     for (i = 0, il = sels.length; i < il; ++i) {
       var child = sels[i];
       if (WT.hasTag(child, 'LI')) {
	 var a = child.firstChild;
         if (child.orig == null) {
           child.orig = a.firstChild.innerHTML;
	 }

	 var result = matcher(child.orig),
	     match = showall || result.match;

	 if (result.suggestion != a.firstChild.innerHTML)
	   a.firstChild.innerHTML = result.suggestion;

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
	 self.showPopup(edit);
	 sel = null;
       }

       if (!sel || (sel.style.display == 'none')) {
	 sel = toselect || first ;
	 sel.parentNode.scrollTop = 0;
         selId = sel.id;
       }

       sel.className = 'active';
       scrollToSelected(sel);
     }
   };

   this.editKeyUp = function(edit, event) {
     if (editId == null)
       return;

     if (!checkEdit(edit))
       return;

     if (!visible()
	 && (event.keyCode == key_enter
	     || event.keyCode == key_tab)) {
       return;
     }

     if (event.keyCode == key_escape
         || event.keyCode == key_left
         || event.keyCode == key_right) {
       hidePopup();
     } else {
       if (edit.value != lastFilterValue) {
		 editId = edit.id;
		 self.refilter(edit.value);
	   } else {
	 var sel = selId ? WT.getElement(selId) : null;
	 if (sel)
	   scrollToSelected(sel);
       }
     }
   };

   el.onclick = contentClicked;

   /*
    * In Safari, scrolling causes the edit to lose focus, but we don't want
    * that. Can it be avoided? In any case, this fixes it.
    */
   el.onscroll = function() {
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
(2, JavaScriptConstructor, "WSuggestionPopupStdMatcher",
 function(highlightBeginTag, highlightEndTag, listSeparator, whiteSpace,
	  wordSeparators, wordRegexp, appendReplacedText) {
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

     var regexp;
     if (wordRegexp.length == 0) {
       if (wordSeparators.length != 0) {
         regexp = "(^|(?:[";
         for (var i = 0; i < wordSeparators.length; ++i) {
           var hexCode = wordSeparators.charCodeAt(i).toString(16);
           while (hexCode.length < 4)
             hexCode = "0" + hexCode;
           regexp += "\\u" + hexCode;
         }
         regexp += "]))";
       } else
	 regexp = "(^)";
     } else
       regexp = "(" + wordRegexp + ")";

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
