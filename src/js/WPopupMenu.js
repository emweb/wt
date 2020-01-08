/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER
(1, JavaScriptConstructor, "WPopupMenu",
 function(APP, el, autoHideDelay) {
   el.wtObj = this;

   var self = this,
       WT = APP.WT,
       hideTimeout = null,
       entered = false,
       current = null,
       touch = null,
       haveMouseDown = false;

   if (WT.isIOS) {
     $(el).bind('touchstart', startElTouch);
     $(el).bind('touchend', endElTouch);
   }

   function doHide() {
     setOthersInactive(el, null);
     el.style.display = 'none';
	 setTimeout(function() {
       APP.emit(el.id, 'cancel');
	 }, 0);
   }

   function setActive(item, active) {
     $(item).toggleClass("active", active);
   }

   function submenu(item) {
     if (item.subMenu)
       return item.subMenu;
     else {
       var u = item.lastChild;
       if (u && WT.hasTag(u, "UL")) {
	 item.subMenu = u;
	 u.parentItem = item;

	 $(u).mousemove(handleSubMenus);

	 bindOverEvents(u);

	 return u;
       } else
	 return null;
     }
   }

   function showSubmenu(menu) {
     menu.style.display='block';
     if (menu.parentNode == menu.parentItem) {
       menu.parentNode.removeChild(menu);
       el.parentNode.appendChild(menu);
     }
     /*
      * we actually want to align the first item, so we need to adjust
      * for the menu padding and border
      */
     var margin =  WT.px(menu, 'paddingTop') + WT.px(menu, 'borderTopWidth')
     WT.positionAtWidget(menu.id, menu.parentItem.id, WT.Horizontal, -margin);
     setOthersInactive(menu, null);

     if (WT.isIOS) {
       $(menu).unbind('touchstart', startElTouch).bind('touchstart', startElTouch);
       $(menu).unbind('touchend', endElTouch).bind('touchend', endElTouch);
     }
   }

   function setOthersInactive(topLevelMenu, activeItem) {
     function itemLeadsTo(item1, item2) {
       if (item1 == item2)
	 return true;
       else if (item2) {
	 var parent = item2.parentNode.parentItem;
	 if (parent)
	   return itemLeadsTo(item1, parent);
	 else
	   return false;
       } else
	 return false;
     }

     function processMenu(menu) {
       var i, il;

       for (i = 0, il = menu.childNodes.length; i < il; ++i) {
	 var item = menu.childNodes[i];

	 if (!itemLeadsTo(item, activeItem)) {
	   setActive(item, false);
	   var sm = submenu(item);
	   if (sm) {
	     sm.style.display = 'none';
	     processMenu(sm);
	   }
	 } else if (item !== activeItem) {
	   var sm = submenu(item);
	   if (sm)
	     processMenu(sm);
	 }
       }
     }

     processMenu(topLevelMenu);
   }

   function handleSubMenus(e) {
     /*
      * If mouse is over an item that has a sub menu, then show the sub menu
      * if it is not already shown
      */
     var item = WT.target(e);

     while (item && !WT.hasTag(item, "LI") && !WT.hasTag(item, "UL"))
       item = item.parentNode;

     if (WT.hasTag(item, "LI")) {
       if (item === current)
	 return;

       current = item;

       setActive(item, true);

       var menu = submenu(item);
       if (menu)
	 showSubmenu(menu);

       /*
	* collapse all submenus not leading to this item
	*/
       setOthersInactive(el, item);
     }
   }

   function mouseLeave() {
     entered = false;
     clearTimeout(hideTimeout);
     if (autoHideDelay >= 0)
       hideTimeout = setTimeout(doHide, autoHideDelay);
   }

   function mouseEnter() {
     entered = true;
     clearTimeout(hideTimeout);
   }

   function bindOverEvents(popup) {
     $(popup).mouseleave(mouseLeave).mouseenter(mouseEnter);
   }

   function stillExist() {
	 return document.getElementById(el.id) != null;
   }

   function onDocumentDown(event) {
     haveMouseDown = true;
     if (stillExist() && WT.button(event) != 1)
       doHide();
   }

   function onDocumentClick(event) {
     if (stillExist()) {
       haveMouseDown = false;
       doHide();
     }
   }

   function onDocumentKeyDown(event) {
     if (stillExist() && event.keyCode == 27)
       doHide();
   }

   this.setHidden = function(hidden) {
     if (!hidden)
       haveMouseDown = false; // cancel pending hide

     if (hideTimeout) {
       clearTimeout(hideTimeout);
       hideTimeout = null;
     }

     entered = false;
     current = null;

     if (hidden) {
       el.style.position = '';
       el.style.display = '';
       el.style.left = '';
       el.style.top = '';
       $(document).unbind('mousedown', onDocumentDown);     
       unbindDocumentClick();
       $(document).unbind('keydown', onDocumentKeyDown);     
     } else {
       setTimeout(function() {
	   $(document).bind('mousedown', onDocumentDown);
           bindDocumentClick();
	   $(document).bind('keydown', onDocumentKeyDown);
	 }, 0);
       el.style.display = 'block';
     }

     setOthersInactive(el, null);
   };

   this.popupAt = function(widget) {
     bindOverEvents(widget);
   };

   function bindDocumentClick() {
     if (WT.isIOS) {
       $(document).bind("touchstart", startTouch);
       $(document).bind("touchend", endTouch);
     } else
       $(document).bind("click", onDocumentClick);
   }

   function unbindDocumentClick() {
     if (WT.isIOS) {
       $(document).unbind("touchstart", startTouch);
       $(document).unbind("touchend", endTouch);
     } else
       $(document).unbind("click", onDocumentClick);
   }

   function startTouch(event) {
     var l = event.originalEvent.touches;
     if (l.length > 1)
       touch = null;
     else {
       touch = {
         x: l[0].screenX,
         y: l[0].screenY
       }
     }
   }

   function startElTouch(event) {
     event.stopPropagation();
   }

   function endTouch(event) {
     if (touch) {
       var t = event.originalEvent.changedTouches[0];
       if (Math.abs(touch.x - t.screenX) < 20 && Math.abs(touch.y - t.screenY) < 20)
         onDocumentClick(event);
       touch = null;
     }
   }

   function endElTouch(event) {
     event.stopPropagation();
   }

   setTimeout(function() { bindOverEvents(el); }, 0);

   $(el).mousemove(handleSubMenus);
 });
