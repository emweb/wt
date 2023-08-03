/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER(
  1,
  JavaScriptConstructor,
  "WSuggestionPopup",
  function(
    APP,
    el,
    replacerJS,
    matcherJS,
    filterMinLength,
    filterMore,
    defaultValue,
    isDropDownIconUnfiltered,
    autoSelectEnabled
  ) {
    el.wtObj = this;

    const self = this;
    const WT = APP.WT;

    const key_tab = 9;
    const key_enter = 13;
    const key_escape = 27;

    const key_pup = 33;
    const key_pdown = 34;
    const key_left = 37;
    const key_up = 38;
    const key_right = 39;
    const key_down = 40;

    let selId = null,
      editId = null,
      kd = false,
      filter = null,
      filtering = null,
      filterPartial = filterMore,
      delayHideTimeout = null,
      lastFilterValue = null,
      droppedDown = false;

    this.defaultValue = defaultValue;

    function isBS5() {
      return typeof WT.theme === "object" &&
        WT.theme.type === "bootstrap" &&
        WT.theme.version === 5;
    }

    /* Checks if we are (still) assisting the given edit */
    function checkEdit(edit) {
      return edit.classList.contains("Wt-suggest-onedit") ||
        edit.classList.contains("Wt-suggest-dropdown");
    }

    function visible() {
      return el.style.display !== "none";
    }

    function positionPopup(edit) {
      el.style.display = "block";
      WT.positionAtWidget(el.id, edit.id, WT.Vertical);
    }

    function contentClicked(event) {
      const e = event || window.event;
      let line = WT.target(e);
      if (WT.hasTag(line, "UL")) {
        return;
      }

      while (line && !WT.hasTag(line, "LI")) {
        line = line.parentNode;
      }

      if (line) {
        suggestionClicked(line);
      }
    }

    function suggestionClicked(line) {
      const suggestion = line.firstChild.firstChild,
        edit = WT.getElement(editId),
        sText = suggestion.innerHTML,
        sValue = suggestion.getAttribute("sug");

      edit.focus();
      replacerJS(edit, sText, sValue);

      APP.emit(el, "select", line.id, edit.id);

      hidePopup();

      editId = null;
    }

    let keyDownFun = null;

    this.showPopup = function(edit) {
      el.style.display = "block";
      self.bringToFront();
      selId = null;
      lastFilterValue = null;
      keyDownFun = edit.onkeydown;
      edit.onkeydown = function(event) {
        const e = event || window.event, o = this;
        self.editKeyDown(o, e);
      };
    };

    this.bringToFront = function() {
      const maxz = WT.maxZIndex();
      if (maxz > el.style["zIndex"]) {
        const newZIndex = maxz + 1;
        el.style["zIndex"] = newZIndex;
      }
    };

    function hidePopup() {
      el.style.display = "none";
      if (editId !== null && keyDownFun !== null) {
        const edit = WT.getElement(editId);
        edit.onkeydown = keyDownFun;
        keyDownFun = null;
      }
    }

    function calcButtonWidth(edit) {
      if (isBS5()) {
        try {
          const style = getComputedStyle(edit);
          const widthPx = parseInt(style.backgroundSize.match(/^([0-9]+)px ([0-9]+)px$/)[1], 10);
          const rightOffsetPx = parseInt(style.backgroundPositionX.match(/^calc[(]100% - ([0-9]+)px[)]$/)[1], 10);
          return widthPx + rightOffsetPx;
        } catch (e) {
          return 28;
        }
      } else {
        return 16;
      }
    }

    this.editMouseMove = function(edit, event) {
      if (!checkEdit(edit)) {
        return;
      }

      const xy = WT.widgetCoordinates(edit, event);
      if (xy.x > edit.offsetWidth - calcButtonWidth(edit)) {
        if (edit.classList) {
          edit.classList.add("Wt-suggest-dropdown-hover");
        }
        edit.style.cursor = "default";
      } else {
        if (edit.classList) {
          edit.classList.remove("Wt-suggest-dropdown-hover");
        }
        edit.style.cursor = "";
      }
    };

    this.showAt = function(edit, value) {
      hidePopup();
      editId = edit.id;
      droppedDown = true;
      if (typeof value === "undefined") {
        value = "";
      }
      self.refilter(value);
    };

    this.editClick = function(edit, event) {
      if (!checkEdit(edit)) {
        return;
      }

      const xy = WT.widgetCoordinates(edit, event);
      if (xy.x > edit.offsetWidth - calcButtonWidth(edit)) {
        if (editId !== edit.id || !visible()) {
          self.showAt(edit, "");
        } else {
          hidePopup();
          editId = null;
        }
      }
    };

    function first(down) {
      const sels = el.childNodes;
      for (let n = down ? sels[0] : sels[sels.length - 1]; n; n = down ? n.nextSibling : n.previousSibling) {
        if (WT.hasTag(n, "LI") && n.style.display !== "none") {
          return n;
        }
      }

      return null;
    }

    function next(n, down) {
      for (n = down ? n.nextSibling : n.previousSibling; n; n = down ? n.nextSibling : n.previousSibling) {
        if (WT.hasTag(n, "LI")) {
          if (n.style.display !== "none") {
            return n;
          }
        }
      }

      return null;
    }

    this.editKeyDown = function(edit, event) {
      if (!checkEdit(edit)) {
        return true;
      }

      if (editId !== edit.id) {
        if (edit.classList.contains("Wt-suggest-onedit")) {
          editId = edit.id;
          droppedDown = false;
        } else if (
          edit.classList.contains("Wt-suggest-dropdown") &&
          event.keyCode === key_down
        ) {
          editId = edit.id;
          droppedDown = true;
        } else {
          editId = null;
          return true;
        }
      }

      if (visible()) {
        const sel = selId ? WT.getElement(selId) : null;

        if ((event.keyCode === key_enter) || (event.keyCode === key_tab)) {
          /*
          * Select currently selectd
          */
          if (sel) {
            suggestionClicked(sel);
            WT.cancelEvent(event);
            setTimeout(function() {
              edit.focus();
            }, 0);
          } else {
            hidePopup();
          }
          return false;
        } else if (
          event.keyCode === key_down ||
          event.keyCode === key_up ||
          event.keyCode === key_pdown ||
          event.keyCode === key_pup
        ) {
          /*
          * Handle navigation in list
          */
          if (event.type.toUpperCase() === "KEYDOWN") {
            kd = true;
            WT.cancelEvent(event, WT.CancelDefaultAction);
          }

          if (event.type.toUpperCase() === "KEYPRESS" && kd === true) {
            WT.cancelEvent(event);
            return false;
          }

          /*
          * Find next selected node
          */
          let n = sel;
          const down = event.keyCode === key_down || event.keyCode === key_pdown;
          if (!n) {
            n = first(down);
            scrollToSelected(n);
          } else {
            const count = (event.keyCode === key_pdown || event.keyCode === key_pup ?
              el.clientHeight / sel.offsetHeight :
              1);

            for (let i = 0; n && i < count; ++i) {
              const l = next(n, down);
              if (!l && autoSelectEnabled) {
                break;
              }
              n = l;
            }
          }

          /*
          * Update selection
          */
          if (sel) {
            sel.classList.remove("active");
            if (isBS5()) {
              sel.firstChild.classList.remove("active");
            }
            selId = null;
          }
          if (n && WT.hasTag(n, "LI")) {
            n.classList.add("active");
            if (isBS5()) {
              n.firstChild.classList.add("active");
            }
            scrollToSelected(n);
            selId = n.id;
          }

          return false;
        }
      }
      return (event.keyCode !== key_enter && event.keyCode !== key_tab);
    };

    this.filtered = function(f, partial) {
      filter = f;
      filterPartial = partial;
      self.refilter(lastFilterValue);
    };

    function scrollToSelected(sel) {
      const p = sel.parentNode;

      if (sel.offsetTop + sel.offsetHeight > p.scrollTop + p.clientHeight) {
        p.scrollTop = sel.offsetTop + sel.offsetHeight - p.clientHeight;
      } else if (sel.offsetTop < p.scrollTop) {
        p.scrollTop = sel.offsetTop;
      }
    }

    /*
    * Refilter the current selection list based on the edit value.
    */
    this.refilter = function(value) {
      if (!editId) {
        // If edit is null we probably have already choosen a suggestion and
        // we therefore don't need to refilter!
        return;
      }

      let sel = selId ? WT.getElement(selId) : null;
      const edit = WT.getElement(editId),
        matcher = matcherJS(edit),
        sels = el.childNodes,
        text = (isDropDownIconUnfiltered && value !== null) ? value : matcher(null);
      lastFilterValue = isDropDownIconUnfiltered ? value : edit.value;

      if (filterMinLength > 0 || filterMore) {
        if (text.length < filterMinLength && !droppedDown) {
          hidePopup();
          return;
        } else {
          const nf = filterPartial ?
            text :
            text.substring(0, Math.max(filter !== null ? filter.length : 0, filterMinLength));

          if (nf !== filter) {
            if (nf !== filtering) {
              filtering = nf;
              APP.emit(el, "filter", nf);
            }
          }
        }
      }

      let first = null, toselect = null;
      const showall = droppedDown && text.length === 0;

      for (let i = 0, il = sels.length; i < il; ++i) {
        const child = sels[i];
        if (WT.hasTag(child, "LI")) {
          const a = child.firstChild;
          if (child.orig === null || typeof child.orig === "undefined") {
            child.orig = a.firstChild.innerHTML;
          }

          const result = matcher(child.orig),
            match = showall || result.match;

          if (result.suggestion !== a.firstChild.innerHTML) {
            a.firstChild.innerHTML = result.suggestion;
          }

          if (match) {
            if (child.style.display !== "") {
              child.style.display = "";
            }
            if (first === null) {
              first = child;
            }
            if (i === this.defaultValue) {
              toselect = child;
            }
          } else if (child.style.display !== "none") {
            child.style.display = "none";
          }

          child.classList.remove("active");
          if (isBS5()) {
            child.firstChild.classList.remove("active");
          }
        }
      }

      if (first === null) {
        hidePopup();
      } else {
        if (!visible()) {
          positionPopup(edit);
          self.showPopup(edit);
          sel = null;
        }

        if ((autoSelectEnabled || toselect) && (!sel || (sel.style.display === "none"))) {
          sel = toselect || first;
          sel.parentNode.scrollTop = 0;
          selId = sel.id;
        }

        if (sel) {
          sel.classList.add("active");
          if (isBS5()) {
            sel.firstChild.classList.add("active");
          }
          scrollToSelected(sel);
        }
      }
    };

    this.editKeyUp = function(edit, event) {
      if (editId === null) {
        return;
      }

      if (!checkEdit(edit)) {
        return;
      }

      if (
        !visible() &&
        (event.keyCode === key_enter ||
          event.keyCode === key_tab)
      ) {
        return;
      }

      if (
        event.keyCode === key_escape ||
        event.keyCode === key_left ||
        event.keyCode === key_right
      ) {
        hidePopup();
      } else if (
        event.keyCode === key_down ||
        event.keyCode === key_up ||
        event.keyCode === key_pdown ||
        event.keyCode === key_pup
      ) {
        // do nothing
      } else {
        if (edit.value !== lastFilterValue) {
          editId = edit.id;
          self.refilter(edit.value);
        } else {
          const sel = selId ? WT.getElement(selId) : null;
          if (sel) {
            scrollToSelected(sel);
          }
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
        const edit = WT.getElement(editId);
        if (edit) {
          edit.focus();
        }
      }
    };

    this.delayHide = function(edit, _event) {
      delayHideTimeout = setTimeout(function() {
        delayHideTimeout = null;
        if (el && (edit === null || editId === edit.id)) {
          hidePopup();
        }
      }, 300);
    };
  }
);

WT_DECLARE_WT_MEMBER(
  2,
  JavaScriptConstructor,
  "WSuggestionPopupStdMatcher",
  function(
    highlightBeginTag,
    highlightEndTag,
    listSeparator,
    whiteSpace,
    wordSeparators,
    wordRegexp,
    appendReplacedText
  ) {
    function parseEdit(edit) {
      const value = edit.value;
      const pos = edit.selectionStart ? edit.selectionStart : value.length;

      let start = listSeparator ?
        value.lastIndexOf(listSeparator, pos - 1) + 1 :
        0;

      while (
        (start < pos) &&
        (whiteSpace.indexOf(value.charAt(start)) !== -1)
      ) {
        ++start;
      }

      return { start: start, end: pos };
    }

    this.match = function(edit) {
      const range = parseEdit(edit);
      const value = edit.value.substring(range.start, range.end);

      let regexp;
      if (wordRegexp.length === 0) {
        if (wordSeparators.length !== 0) {
          regexp = "(^|(?:[";
          for (let i = 0; i < wordSeparators.length; ++i) {
            let hexCode = wordSeparators.charCodeAt(i).toString(16);
            while (hexCode.length < 4) {
              hexCode = "0" + hexCode;
            }
            regexp += "\\u" + hexCode;
          }
          regexp += "]))";
        } else {
          regexp = "(^)";
        }
      } else {
        regexp = "(" + wordRegexp + ")";
      }

      regexp += "(" + value.replace(new RegExp("([\\^\\\\\\][\\-.$*+?()|{}])", "g"), "\\$1") +
        ")";

      regexp = new RegExp(regexp, "gi");

      return function(suggestion) {
        if (!suggestion) {
          return value;
        }

        let matched = false;

        if (value.length) {
          const highlighted = suggestion.replace(
            regexp,
            "$1" + highlightBeginTag + "$2" +
              highlightEndTag
          );
          if (highlighted !== suggestion) {
            matched = true;
            suggestion = highlighted;
          }
        }

        return { match: matched, suggestion: suggestion };
      };
    };

    this.replace = function(edit, suggestionText, suggestionValue) {
      const range = parseEdit(edit);

      let nv = edit.value.substring(0, range.start) + suggestionValue +
        appendReplacedText;

      if (range.end < edit.value.length) {
        nv += edit.value.substring(range.end, edit.value.length);
      }

      edit.value = nv;

      if (edit.selectionStart) {
        edit.selectionStart = range.start + suggestionValue.length +
          appendReplacedText.length;
        edit.selectionEnd = edit.selectionStart;
      }
    };
  }
);
