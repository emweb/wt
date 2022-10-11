/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER(
  1,
  JavaScriptConstructor,
  "WSpinBox",
  function(APP, edit, precision, prefix, suffix, minValue, maxValue, stepValue, decimalPoint, groupSeparator) {
    const NaNError = "Must be a number";
    const tooSmallError = "The number must be at least ";
    const tooLargeError = "The number may be at most ";

    const CLASS_DOWN = "dn";
    const CLASS_UP = "up";
    const CLASS_UNSELECTABLE = "unselectable";

    edit.wtObj = this;

    const self = this, WT = APP.WT, key_up = 38, key_down = 40, CH = "crosshair";

    let dragStartXY = null, dragStartValue, changed = false;
    let validator = null;
    let isDoubleSpinBox = false;
    let wrapAround = false;

    function isReadOnly() {
      return edit.readOnly;
    }

    function getValue() {
      const lineEdit = edit.wtLObj;
      let v = "";
      if (typeof lineEdit !== "undefined") {
        v = lineEdit.getValue();
        if (v === "") {
          v = prefix + "0" + suffix;
        }
      } else {
        v = edit.value;
      }
      if (v.startsWith(prefix)) {
        v = v.substring(prefix.length);
        if (v.endsWith(suffix)) {
          v = v.substring(0, v.length - suffix.length);
          if (groupSeparator) {
            v = v.split(groupSeparator).join("");
          }
          v = v.replace(decimalPoint, ".");
          return Number(v);
        }
      }

      return null;
    }

    function setValue(v) {
      const lineEdit = edit.wtLObj;
      if (v > maxValue) {
        if (wrapAround) {
          const range = maxValue - minValue;
          v = minValue + ((v - minValue) % (range + 1));
        } else {
          v = maxValue;
        }
      } else if (v < minValue) {
        if (wrapAround) {
          const range = maxValue - minValue;
          v = maxValue - ((Math.abs(v - minValue) - 1) % (range + 1));
        } else {
          v = minValue;
        }
      }

      let newValue = v.toFixed(precision);
      newValue = newValue.replace(".", decimalPoint);
      const dotPos = newValue.indexOf(decimalPoint);
      let result = "";
      if (dotPos !== -1) {
        for (let i = 0; i < dotPos; i++) {
          result += newValue.charAt(i);
          if (i < dotPos - 1 && ((dotPos - i - 1) % 3 === 0)) {
            result += groupSeparator;
          }
        }
        result += newValue.substr(dotPos);
      } else {
        result = newValue;
      }

      const oldv = edit.value;
      if (typeof lineEdit !== "undefined") {
        lineEdit.setValue(prefix + result + suffix);
      } else {
        edit.value = prefix + result + suffix;
      }

      changed = true;
      self.jsValueChanged(oldv, v);
    }

    function inc() {
      let v = getValue();
      if (v !== null) {
        v += stepValue;
        setValue(v);
      }
    }

    function dec() {
      let v = getValue();
      if (v !== null) {
        v -= stepValue;
        setValue(v);
      }
    }

    this.setIsDoubleSpinBox = function(isDouble) {
      isDoubleSpinBox = isDouble;

      this.configure(precision, prefix, suffix, minValue, maxValue, stepValue);
    };

    this.setWrapAroundEnabled = function(enabled) {
      wrapAround = enabled;
    };

    this.configure = function(newPrecision, newPrefix, newSuffix, newMinValue, newMaxValue, newStepValue) {
      precision = newPrecision;
      prefix = newPrefix;
      suffix = newSuffix;
      minValue = newMinValue;
      maxValue = newMaxValue;
      stepValue = newStepValue;

      const useDoubleValidator = (isDoubleSpinBox ||
        typeof WT.WIntValidator === "undefined");
      if (useDoubleValidator) {
        validator = new WT.WDoubleValidator(
          true,
          false,
          minValue,
          maxValue,
          ".",
          "",
          NaNError,
          NaNError,
          tooSmallError + minValue,
          tooLargeError + maxValue
        );
      } else {
        validator = new WT.WIntValidator(
          true,
          minValue,
          maxValue,
          "",
          NaNError,
          NaNError,
          tooSmallError + minValue,
          tooLargeError + maxValue
        );
      }
    };

    this.mouseOut = function(_o, _event) {
      edit.classList.remove(CLASS_DOWN, CLASS_UP);
    };

    this.mouseMove = function(o, event) {
      if (isReadOnly()) {
        return;
      }

      if (!dragStartXY) {
        const xy = WT.widgetCoordinates(edit, event);

        if (edit.classList.contains(CLASS_DOWN) || edit.classList.contains(CLASS_UP)) {
          edit.classList.remove(CLASS_DOWN, CLASS_UP);
        }

        let bootstrapVersion = -1;
        if (
          typeof WT.theme === "object" &&
          WT.theme.type === "bootstrap"
        ) {
          bootstrapVersion = WT.theme.version;
        }
        if (
          bootstrapVersion >= 4 &&
          xy.x > edit.offsetWidth - 30 &&
          xy.x < edit.offsetWidth - 10
        ) {
          const mid = edit.offsetHeight / 2;
          if (xy.y >= mid - 3 && xy.y <= mid + 3) {
            edit.style.cursor = CH;
          } else {
            edit.style.cursor = "default";
            if (xy.y < mid - 1) {
              edit.classList.add(CLASS_UP);
            } else {
              edit.classList.add(CLASS_DOWN);
            }
          }
        } else if (bootstrapVersion < 4 && xy.x > edit.offsetWidth - 22) {
          const mid = edit.offsetHeight / 2;
          if (xy.y >= mid - 3 && xy.y <= mid + 3) {
            edit.style.cursor = CH;
          } else {
            edit.style.cursor = "default";
            if (xy.y < mid - 1) {
              edit.classList.add(CLASS_UP);
            } else {
              edit.classList.add(CLASS_DOWN);
            }
          }
        } else {
          if (edit.style.cursor !== "") {
            edit.style.cursor = "";
          }
        }
      } else {
        const dy = WT.pageCoordinates(event).y - dragStartXY.y;
        let v = dragStartValue;
        if (v !== null) {
          v = v - dy * stepValue;
          setValue(v);
        }
      }
    };

    this.mouseDown = function(o, event) {
      WT.capture(null);
      if (isReadOnly()) {
        return;
      }

      if (edit.style.cursor === CH) {
        WT.capture(null);
        WT.capture(edit);
        edit.classList.add(CLASS_UNSELECTABLE);

        dragStartXY = WT.pageCoordinates(event);
        dragStartValue = getValue();
      } else {
        const xy = WT.widgetCoordinates(edit, event);
        let bootstrapVersion = -1;
        if (
          typeof WT.theme === "object" &&
          WT.theme.type === "bootstrap"
        ) {
          bootstrapVersion = WT.theme.version;
        }
        if (bootstrapVersion >= 5 && xy.x > edit.offsetWidth - 30 && xy.x < edit.offsetWidth - 10) {
          // suppress selection, focus
          WT.cancelEvent(event);
          WT.capture(edit);
          edit.classList.add(CLASS_UNSELECTABLE);

          const mid = edit.offsetHeight / 2;
          if (xy.y < mid) {
            WT.eventRepeat(function() {
              inc();
            });
          } else {
            WT.eventRepeat(function() {
              dec();
            });
          }
        } else if (bootstrapVersion < 4 && xy.x > edit.offsetWidth - 22) {
          // suppress selection, focus
          WT.cancelEvent(event);
          WT.capture(edit);
          edit.classList.add(CLASS_UNSELECTABLE);

          const mid = edit.offsetHeight / 2;
          if (xy.y < mid) {
            WT.eventRepeat(function() {
              inc();
            });
          } else {
            WT.eventRepeat(function() {
              dec();
            });
          }
        }
      }
    };

    this.mouseUp = function(o, _event) {
      edit.classList.remove(CLASS_UNSELECTABLE);
      if (isReadOnly()) {
        return;
      }

      if (changed || dragStartXY !== null) {
        dragStartXY = null;
        changed = false;
        o.onchange();
      }

      WT.stopRepeat();
    };

    this.keyDown = function(o, event) {
      if (isReadOnly()) {
        return;
      }

      if (event.keyCode === key_down) {
        WT.eventRepeat(function() {
          dec();
        });
      } else if (event.keyCode === key_up) {
        WT.eventRepeat(function() {
          inc();
        });
      }
    };

    this.keyUp = function(o, _event) {
      if (isReadOnly()) {
        return;
      }

      if (changed) {
        changed = false;
        o.onchange();
      }

      WT.stopRepeat();
    };

    this.setLocale = function(point, separator) {
      decimalPoint = point;
      groupSeparator = separator;
    };

    /*
    * Customized validation function, called from WFormWidget
    */
    this.validate = function(_text) {
      let v = getValue();

      if (v === null) {
        v = "a"; // NaN
      }

      return validator.validate(v);
    };

    this.jsValueChanged = function() {};

    this.setIsDoubleSpinBox(isDoubleSpinBox);
  }
);
