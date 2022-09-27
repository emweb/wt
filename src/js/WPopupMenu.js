/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER(1, JavaScriptConstructor, "WPopupMenu", function(APP, el, autoHideDelay) {
  el.wtObj = this;

  const WT = APP.WT;
  let hideTimeout = null,
    current = null,
    touch = null;

  if (WT.isIOS) {
    el.addEventListener("touchstart", startElTouch);
    el.addEventListener("touchend", endElTouch);
  }

  function doHide() {
    setOthersInactive(el, null);
    el.style.display = "none";
    setTimeout(function() {
      APP.emit(el.id, "cancel");
    }, 0);
  }

  function setActive(item, active) {
    item.classList.toggle("active", active);
  }

  function submenu(item) {
    if (item.subMenu) {
      return item.subMenu;
    } else {
      const u = item.lastChild;
      if (u && WT.hasTag(u, "UL")) {
        item.subMenu = u;
        u.parentItem = item;

        u.addEventListener("mousemove", handleSubMenus);

        bindOverEvents(u);

        return u;
      } else {
        return null;
      }
    }
  }

  function showSubmenu(menu) {
    menu.style.display = "block";
    if (menu.parentNode === menu.parentItem) {
      menu.parentNode.removeChild(menu);
      el.parentNode.appendChild(menu);
    }
    /*
      * we actually want to align the first item, so we need to adjust
      * for the menu padding and border
      */
    const margin = WT.px(menu, "paddingTop") + WT.px(menu, "borderTopWidth");
    WT.positionAtWidget(menu.id, menu.parentItem.id, WT.Horizontal, -margin);
    setOthersInactive(menu, null);

    if (WT.isIOS) {
      menu.removeEventListener("touchstart", startElTouch);
      menu.addEventListener("touchstart", startElTouch);
      menu.removeEventListener("touchend", endElTouch);
      menu.addEventListener("touchend", endElTouch);
    }
  }

  function setOthersInactive(topLevelMenu, activeItem) {
    function itemLeadsTo(item1, item2) {
      if (item1 === item2) {
        return true;
      } else if (item2) {
        const parent = item2.parentNode.parentItem;
        if (parent) {
          return itemLeadsTo(item1, parent);
        } else {
          return false;
        }
      } else {
        return false;
      }
    }

    function processMenu(menu) {
      for (const item of menu.childNodes) {
        if (!itemLeadsTo(item, activeItem)) {
          setActive(item, false);
          const sm = submenu(item);
          if (sm) {
            sm.style.display = "none";
            processMenu(sm);
          }
        } else if (item !== activeItem) {
          const sm = submenu(item);
          if (sm) {
            processMenu(sm);
          }
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
    let item = WT.target(e);

    while (item && !WT.hasTag(item, "LI") && !WT.hasTag(item, "UL")) {
      item = item.parentNode;
    }

    if (WT.hasTag(item, "LI")) {
      if (item === current) {
        return;
      }

      current = item;

      setActive(item, true);

      const menu = submenu(item);
      if (menu) {
        showSubmenu(menu);
      }

      /*
        * collapse all submenus not leading to this item
        */
      setOthersInactive(el, item);
    }
  }

  function mouseLeave() {
    clearTimeout(hideTimeout);
    if (autoHideDelay >= 0) {
      hideTimeout = setTimeout(doHide, autoHideDelay);
    }
  }

  function mouseEnter() {
    clearTimeout(hideTimeout);
  }

  function bindOverEvents(popup) {
    popup.addEventListener("mouseleave", mouseLeave);
    popup.addEventListener("mouseenter", mouseEnter);
  }

  function stillExist() {
    return document.getElementById(el.id) !== null;
  }

  function onDocumentDown(event) {
    if (stillExist() && WT.button(event) !== 1) {
      doHide();
    }
  }

  function onDocumentClick() {
    if (stillExist()) {
      doHide();
    }
  }

  function onDocumentKeyDown(event) {
    if (stillExist() && event.keyCode === 27) {
      doHide();
    }
  }

  this.setHidden = function(hidden) {
    if (hideTimeout) {
      clearTimeout(hideTimeout);
      hideTimeout = null;
    }

    current = null;

    if (hidden) {
      el.style.position = "";
      el.style.display = "";
      el.style.left = "";
      el.style.top = "";
      document.removeEventListener("mousedown", onDocumentDown);
      unbindDocumentClick();
      document.removeEventListener("keydown", onDocumentKeyDown);
    } else {
      setTimeout(function() {
        document.addEventListener("mousedown", onDocumentDown);
        bindDocumentClick();
        document.addEventListener("keydown", onDocumentKeyDown);
      }, 0);
      el.style.display = "block";
    }

    setOthersInactive(el, null);
  };

  this.popupAt = function(widget) {
    bindOverEvents(widget);
  };

  function bindDocumentClick() {
    if (WT.isIOS) {
      document.addEventListener("touchstart", startTouch);
      document.addEventListener("touchend", endTouch);
    } else {
      document.addEventListener("click", onDocumentClick);
    }
  }

  function unbindDocumentClick() {
    if (WT.isIOS) {
      document.removeEventListener("touchstart", startTouch);
      document.removeEventListener("touchend", endTouch);
    } else {
      document.removeEventListener("click", onDocumentClick);
    }
  }

  function startTouch(event) {
    const l = event.originalEvent.touches;
    if (l.length > 1) {
      touch = null;
    } else {
      touch = {
        x: l[0].screenX,
        y: l[0].screenY,
      };
    }
  }

  function startElTouch(event) {
    event.stopPropagation();
  }

  function endTouch(event) {
    if (touch) {
      const t = event.originalEvent.changedTouches[0];
      if (Math.abs(touch.x - t.screenX) < 20 && Math.abs(touch.y - t.screenY) < 20) {
        onDocumentClick(event);
      }
      touch = null;
    }
  }

  function endElTouch(event) {
    event.stopPropagation();
  }

  setTimeout(function() {
    bindOverEvents(el);
  }, 0);

  el.addEventListener("mousemove", handleSubMenus);
});
