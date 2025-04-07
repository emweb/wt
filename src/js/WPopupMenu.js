/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER(1, JavaScriptConstructor, "WPopupMenu", function(APP, el, globalAutoHideDelay, autoHideBehaviour) {
  el.wtObj = this;

  const WT = APP.WT;
  const AUTO_HIDE_PREFIX = "Wt-AutoHideDelay-";

  const HideAllEnabled = 0;
  const HideAfterLastDisabled = 1;
  const KeepParents = 2;

  let globalAutoHideBehaviour = autoHideBehaviour;

  let globalHideTimeout = null,
    current = null,
    lastOpenedSubmenu = null,
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

        bindOverEvents(u, item);

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
    const margin = WT.px(menu, "paddingTop") + WT.pxComputedStyle(menu, "borderTopWidth");
    WT.positionAtWidget(
      menu.id,
      menu.parentItem.id,
      WT.Horizontal,
      -margin,
      menu.classList.contains("Wt-AdjustX"),
      menu.classList.contains("Wt-AdjustY")
    );
    setOthersInactive(menu, null);
    lastOpenedSubmenu = menu;

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

  function subMostOpenedMenu() {
    while (lastOpenedSubmenu && lastOpenedSubmenu.style.display === "none") {
      lastOpenedSubmenu = lastOpenedSubmenu.parentItem ?
        lastOpenedSubmenu.parentItem.parentNode :
        null;
    }

    return lastOpenedSubmenu;
  }

  function globalMouseLeave() {
    clearTimeout(globalHideTimeout);
    if (globalAutoHideDelay >= 0) {
      globalHideTimeout = setTimeout(function() {
        const submost = subMostOpenedMenu();

        if (
          globalAutoHideBehaviour !== HideAfterLastDisabled ||
          !submost ||
          getAutoHideDelay(submost) >= 0
        ) {
          doHide();
        }
      }, globalAutoHideDelay);
    }
  }

  function globalMouseEnter() {
    clearTimeout(globalHideTimeout);
  }

  function getAutoHideDelay(popup) {
    let autoHideDelay = -1;
    for (const className of popup.classList) {
      if (className.startsWith(AUTO_HIDE_PREFIX)) {
        const delayStr = className.substring(AUTO_HIDE_PREFIX.length);
        const delay = parseInt(delayStr);
        if (!isNaN(delay)) {
          autoHideDelay = delay;
        }
      }
    }

    return autoHideDelay;
  }

  function bindOverEvents(popup, parent = null) {
    let hideTimeout;

    if (!popup.wtObj) {
      popup.wtObj = {};
    }

    popup.wtObj.mouseEnter = function() {
      clearTimeout(hideTimeout);
      if (parent) {
        parent.parentNode.wtObj.mouseEnter();
      }
    };

    popup.wtObj.mouseLeave = function() {
      const autoHideDelay = getAutoHideDelay(popup);
      if (parent) {
        if (autoHideDelay >= 0) {
          function onMouseLeave() {
            const submost = subMostOpenedMenu();
            if (
              popup.style.display !== "none" &&
              (globalAutoHideBehaviour !== HideAfterLastDisabled ||
                !submost ||
                getAutoHideDelay(submost) >= 0)
            ) {
              current = null;
              setOthersInactive(parent.parentNode, null);
            }
          }

          clearTimeout(hideTimeout);
          hideTimeout = setTimeout(onMouseLeave, autoHideDelay);
        }

        if (
          globalAutoHideBehaviour === HideAllEnabled ||
          (globalAutoHideBehaviour === HideAfterLastDisabled && autoHideDelay >= 0)
        ) {
          parent.parentNode.wtObj.mouseLeave();
        }
      }
    };

    if (parent) {
      parent.parentNode.addEventListener("mouseleave", popup.wtObj.mouseLeave);
      parent.parentNode.addEventListener("mouseenter", popup.wtObj.mouseEnter);
      popup.addEventListener("mouseleave", popup.wtObj.mouseLeave);
      popup.addEventListener("mouseenter", popup.wtObj.mouseEnter);
    }

    popup.addEventListener("mouseleave", function() {
      if (globalAutoHideBehaviour !== KeepParents || !parent) {
        globalMouseLeave();
      }
    });
    popup.addEventListener("mouseenter", globalMouseEnter);
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

  this.setAutoHideBehaviour = function(behaviour) {
    globalAutoHideBehaviour = behaviour;
  };

  this.setHidden = function(hidden) {
    if (globalHideTimeout) {
      clearTimeout(globalHideTimeout);
      globalHideTimeout = null;
    }

    current = null;

    if (hidden) {
      el.style.position = "";
      el.style.display = "none";
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
