/*
 * Copyright (C) 2011 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER
(1, JavaScriptConstructor, "WPopupMenu",
 function(APP, el, autoHideDelay) {
   jQuery.data(el, 'obj', this);

   var self = this,
       WT = APP.WT,
       hideTimeout = null,
       location = null,
       entered = 0,
       current = null;

   function doHide() {
     APP.emit(el.id, 'cancel');
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
       }
     }
   }

   function showSubmenu(menu) {
     menu.style.display='block';
     if (menu.parentNode == menu.parentItem) {
       menu.parentNode.removeChild(menu);
       el.parentNode.appendChild(menu);
     }
     WT.positionAtWidget(menu.id, menu.parentItem.id, WT.Horizontal,
			 - WT.px(menu, 'paddingTop'));
     setOthersInactive(menu, null);
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
	   if (sm)
	     sm.style.display = 'none';
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
     --entered;
     if (entered == 0) {
       clearTimeout(hideTimeout);
       if (autoHideDelay >= 0)
	 hideTimeout = setTimeout(doHide, autoHideDelay);
     }
   }

   function mouseEnter() {
     ++entered;
     clearTimeout(hideTimeout);
   }

   function bindOverEvents(popup) {
     $(popup).mouseleave(mouseLeave).mouseenter(mouseEnter);
   }

   function onDocumentClick(event) {
     if (!entered)
       doHide();
   }

   this.setHidden = function(hidden) {
     if (hideTimeout) {
       clearTimeout(hideTimeout);
       hideTimeout = null;
     }

     entered = 0;
     current = null;

     if (autoHideDelay > 0 && !hidden) {
       entered = 1;
       if (!location)   // assume we are currently over the location
	 mouseLeave();
     }

     if (hidden) {
       el.style.position = '';
       el.style.display = '';
       el.style.left = '';
       el.style.top = '';
       $(document).unbind('click', onDocumentClick);     
     } else {
       setTimeout(function() {
		    $(document).bind('click', onDocumentClick);
		  }, 0);
       el.style.display = 'block';
     }

     setOthersInactive(el, null);
   };

   this.popupAt = function(widget) {
     if (location !== widget) {
       location = widget;
       bindOverEvents(location);
       mouseEnter(); // assume we are currently over the location
     }

     self.setHidden(false);
   };

   setTimeout(function() { bindOverEvents(el); }, 0);

   $(el).mousemove(handleSubMenus);
 });
