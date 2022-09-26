/*
 * Copyright (C) 2014 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER(
  1,
  JavaScriptConstructor,
  "WLineEdit",
  function(APP, edit, mask, raw, displayValue, caseMap, spaceChar, flags) {
    const BACKSPACE_KEY = 8;
    const DELETE_KEY = 46;
    const RIGHT_KEY = 39;
    const LEFT_KEY = 37;
    const HOME_KEY = 36;
    const END_KEY = 35;
    const LF_CHAR = 10;
    const CR_CHAR = 13;
    const A_LOWER_CHAR = "a".charCodeAt(0);
    const F_LOWER_CHAR = "f".charCodeAt(0);
    const Z_LOWER_CHAR = "z".charCodeAt(0);
    const A_UPPER_CHAR = "A".charCodeAt(0);
    const F_UPPER_CHAR = "F".charCodeAt(0);
    const Z_UPPER_CHAR = "Z".charCodeAt(0);
    const MINUS_CHAR = "-".charCodeAt(0);
    const PLUS_CHAR = "+".charCodeAt(0);
    const ZERO_CHAR = "0".charCodeAt(0);
    const ONE_CHAR = "1".charCodeAt(0);
    const NINE_CHAR = "9".charCodeAt(0);
    const KEEP_MASK_WHEN_BLURRED_FLAG = 0x1;

    edit.wtLObj = this;

    const self = this, WT = APP.WT;

    function inputSignal() {
      edit.dispatchEvent(new InputEvent("input"));
    }

    function getValue() {
      if (mask === "") {
        return edit.value;
      }
      const value = edit.value;
      let result = "";
      let currentChar = "";
      let i = 0;
      let nbChars = 0;
      for (i = 0; i < value.length; i++) {
        currentChar = value.charAt(i);
        if (currentChar !== spaceChar && mask.charAt(i) !== "_") {
          nbChars += 1;
        }
        if (currentChar !== spaceChar || mask.charAt(i) === "_") {
          result += currentChar;
        }
      }
      if (nbChars > 0) {
        return result;
      } else {
        return "";
      }
    }
    this.getValue = getValue;

    this.setValue = function(newValue) {
      displayValue = newValue;

      if (mask === "") {
        edit.value = newValue;
        return;
      }

      if (!(flags & KEEP_MASK_WHEN_BLURRED_FLAG) && !WT.hasFocus(edit)) {
        edit.value = this.getValue();
        return;
      }

      const selection = WT.getSelectionRange(edit);
      let newCursor = -1;

      edit.value = raw;

      let i = 0;
      let j = 0;
      let charToInsert = "";
      for (; i < newValue.length; i++) {
        charToInsert = newValue.charAt(i);
        if (selection.start === selection.end && selection.start === i) {
          newCursor = j;
        }
        j = insertChar(charToInsert, j, true);
      }

      if (WT.hasFocus(edit)) {
        if (newCursor !== -1) {
          setCursor(newCursor);
        } else if (newValue.length === 0) {
          setCursor(0);
        }
      }
    };

    this.setInputMask = function(newMask, newRaw, newDisplayValue, newCaseMap, newSpaceChar) {
      mask = newMask;
      raw = newRaw;
      caseMap = newCaseMap;
      spaceChar = newSpaceChar;
      self.setValue(newDisplayValue);
    };

    if (mask !== "") {
      this.setInputMask(mask, raw, displayValue, caseMap, spaceChar);
    }

    function skippable(position) {
      return mask.charAt(position) === "_";
    }

    function setCursor(position) {
      WT.setSelectionRange(edit, position, position + 1);
    }

    function moveForward(position) {
      while (skippable(position)) {
        position++;
      }
      setCursor(position);
      return position;
    }

    function moveBackward(position) {
      while (position > 0 && skippable(position)) {
        position--;
      }
      setCursor(position);
      return position;
    }

    function acceptKey(code, position) {
      if (position >= mask.length) {
        return false;
      }
      switch (mask.charAt(position)) {
        case "a":
        case "A": // alphabetical: A-Za-z
          return code >= A_LOWER_CHAR && code <= Z_LOWER_CHAR ||
            code >= A_UPPER_CHAR && code <= Z_UPPER_CHAR;
        case "n":
        case "N": // alphanumeric: A-Za-z0-9
          return code >= A_LOWER_CHAR && code <= Z_LOWER_CHAR ||
            code >= A_UPPER_CHAR && code <= Z_UPPER_CHAR ||
            code >= ZERO_CHAR && code <= NINE_CHAR;
        case "X":
        case "x": // Anything goes
          return true;
        case "0":
        case "9": // 0-9
          return code >= ZERO_CHAR && code <= NINE_CHAR;
        case "d":
        case "D": // 1-9
          return code >= ONE_CHAR && code <= NINE_CHAR;
        case "#": // 0-9, + and -
          return code >= ZERO_CHAR && code <= NINE_CHAR ||
            code === MINUS_CHAR || code === PLUS_CHAR;
        case "h":
        case "H": // hex
          return code >= A_UPPER_CHAR && code <= F_UPPER_CHAR ||
            code >= ZERO_CHAR && code <= NINE_CHAR ||
            code >= A_LOWER_CHAR && code <= F_LOWER_CHAR;
        case "b":
        case "B": // binary
          return code === ZERO_CHAR || code === ONE_CHAR;
      }
      return false;
    }

    function clearSelection(selection) {
      const before = edit.value.substring(0, selection.start);
      const after = edit.value.substring(selection.end);
      const between = raw.substring(selection.start, selection.end);
      edit.value = before + between + after;
      WT.setSelectionRange(edit, selection.start, selection.start);
    }

    function insertChar(charToInsert, position, pasteMode) {
      let j = position;
      const jBefore = j;
      // If the previous static character was typed, don't move forward.
      // This causes an input sequence like "192.168.0.1" to be handled properly.
      if (
        !pasteMode && raw.charAt(j) !== charToInsert &&
        !acceptKey(charToInsert.charCodeAt(0), j) &&
        j - 1 >= 0 && raw.charAt(j - 1) === charToInsert
      ) {
        return j;
      }
      while (
        raw.charAt(j) !== charToInsert &&
        !acceptKey(charToInsert.charCodeAt(0), j) && j < mask.length
      ) {
        j++;
      }
      if (j === mask.length) { // This char didn't fit, discard
        return jBefore;
      }
      const before = edit.value.substring(0, j);
      const after = edit.value.substring(j + 1);
      if (raw.charAt(j) !== charToInsert) {
        if (caseMap.charAt(j) === ">") {
          charToInsert = charToInsert.toUpperCase();
        } else if (caseMap.charAt(j) === "<") {
          charToInsert = charToInsert.toLowerCase();
        }
      }
      edit.value = before + charToInsert + after;
      j++;
      return j;
    }

    this.keyDown = function(o, event) {
      if (mask === "" || edit.readOnly) {
        return;
      }
      const valueBefore = getValue();
      let selection;
      switch (event.keyCode) {
        case RIGHT_KEY:
          WT.cancelEvent(event, WT.CancelDefaultAction);
          selection = WT.getSelectionRange(edit);
          if (selection.end - selection.start <= 1) {
            moveForward(selection.start + 1);
          } else {
            WT.setSelectionRange(edit, selection.end, selection.end);
          }
          break;
        case LEFT_KEY:
          WT.cancelEvent(event, WT.CancelDefaultAction);
          selection = WT.getSelectionRange(edit);
          if (selection.end - selection.start <= 1) {
            moveBackward(selection.start - 1);
          } else {
            WT.setSelectionRange(edit, selection.start, selection.start);
          }
          break;
        case HOME_KEY:
          WT.cancelEvent(event, WT.CancelDefaultAction);
          moveForward(0);
          break;
        case END_KEY:
          WT.cancelEvent(event, WT.CancelDefaultAction);
          WT.setSelectionRange(edit, mask.length, mask.length);
          break;
        case DELETE_KEY:
          WT.cancelEvent(event, WT.CancelDefaultAction);
          selection = WT.getSelectionRange(edit);
          if (selection.end - selection.start <= 1) {
            const position = selection.start;
            if (
              position < mask.length &&
              !skippable(position)
            ) {
              const before = edit.value.substring(0, position);
              const after = edit.value.substring(position + 1);
              edit.value = before + spaceChar + after;
              WT.setSelectionRange(edit, position, position);
            }
          } else {
            clearSelection(selection);
          }
          break;
        case BACKSPACE_KEY:
          WT.cancelEvent(event, WT.CancelDefaultAction);
          selection = WT.getSelectionRange(edit);
          if (selection.end - selection.start <= 1) {
            let position = selection.start - 1;
            if (position >= 0) {
              position = moveBackward(position);
              if (!skippable(position)) {
                const before = edit.value.substring(0, position);
                const after = edit.value.substring(position + 1);
                edit.value = before + spaceChar + after;
                setCursor(position);
              }
            }
          } else {
            clearSelection(selection);
          }
          break;
      }
      if (valueBefore !== getValue()) {
        inputSignal();
      }
    };

    this.keyPressed = function(o, event) {
      if (mask === "" || edit.readOnly) {
        return;
      }
      const valueBefore = getValue();
      const charCode = event.charCode || event.keyCode;
      if (charCode === 0 || charCode === CR_CHAR || charCode === LF_CHAR) {
        return;
      }
      WT.cancelEvent(event, WT.CancelDefaultAction);
      const selection = WT.getSelectionRange(edit);
      if (selection.start < selection.end) {
        clearSelection(selection);
      }
      const currentPosition = insertChar(String.fromCharCode(charCode), selection.start);
      moveForward(currentPosition);
      if (valueBefore !== getValue()) {
        inputSignal();
      }
    };

    let previousValue = this.getValue();

    this.focussed = function(_o, _event) {
      if (mask === "" || edit.readOnly || (flags & KEEP_MASK_WHEN_BLURRED_FLAG)) {
        return;
      }
      previousValue = this.getValue();
      setTimeout(function() {
        self.setValue(displayValue);
      }, 0);
    };

    this.blurred = function(_o, _event) {
      if (mask === "" || edit.readOnly || (flags & KEEP_MASK_WHEN_BLURRED_FLAG)) {
        return;
      }
      displayValue = edit.value;
      edit.value = this.getValue();
      if (edit.value !== previousValue) {
        edit.dispatchEvent(new Event("change", { bubbles: true }));
      }
    };

    this.clicked = function(_o, _event) {
      if (mask === "" || edit.readOnly) {
        return;
      }
      const selection = WT.getSelectionRange(edit);
      if (selection.start === selection.end) {
        setCursor(selection.start);
      }
    };

    /**
     * @param {ClipboardEvent} event
     */
    function paste(event) {
      if (mask === "" || edit.readOnly) {
        return;
      }
      WT.cancelEvent(event, WT.CancelDefaultAction);
      const valueBefore = getValue();
      const selection = WT.getSelectionRange(edit);
      if (selection.start !== selection.end) {
        clearSelection(selection);
      }
      let pasteData = null;
      if (event.clipboardData && event.clipboardData.getData) {
        pasteData = event.clipboardData.getData("text/plain");
      } else {
        if (edit.value !== valueBefore) {
          inputSignal();
        }
        return;
      }
      let charToInsert = "";
      let i = 0, j = selection.start;
      for (; i < pasteData.length; i++) {
        charToInsert = pasteData.charAt(i);
        j = insertChar(charToInsert, j, true);
      }
      moveForward(j);
      if (valueBefore !== getValue()) {
        inputSignal();
      }
    }

    edit.addEventListener("paste", paste, false);

    function cut(event) {
      if (mask === "" || edit.readOnly) {
        return;
      }
      WT.cancelEvent(event, WT.CancelDefaultAction);
      const valueBefore = getValue();
      const selection = WT.getSelectionRange(edit);
      if (selection.start !== selection.end) {
        const cutData = edit.value.substring(selection.start, selection.end);
        if (window.clipboardData && window.clipboardData.setData) {
          window.clipboardData.setData("Text", cutData);
        } else if (event.clipboardData && event.clipboardData.setData) {
          event.clipboardData.setData("text/plain", cutData);
        }
        clearSelection(selection);
      }
      if (valueBefore !== getValue()) {
        inputSignal();
      }
    }

    edit.addEventListener("cut", cut, false);

    function input(_event) {
      if (mask === "" || edit.readOnly) {
        return;
      }
      // When the value was cleared, reset it to "raw".
      // e.g. IE does this when the X button is clicked.
      if (edit.value === "") {
        edit.value = raw;
        moveForward(0);
      }
    }

    edit.addEventListener("input", input, false);

    edit.wtEncodeValue = function() {
      if (mask === "" || (flags & KEEP_MASK_WHEN_BLURRED_FLAG) || WT.hasFocus(edit)) {
        return edit.value;
      } else {
        return displayValue;
      }
    };
  }
);
