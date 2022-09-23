WT_DECLARE_WT_MEMBER(
  1,
  JavaScriptConstructor,
  "WTimeValidator",
  function(mandatory, formats, bottom, top, blankError, formatError, tooSmallError, tooLargeError) {
    this.validate = function(text) {
      if (text.length === 0) {
        if (mandatory) {
          return { valid: false, message: blankError };
        } else {
          return { valid: true };
        }
      }
      /** @type {?[string]} */
      let results = null;
      let h = -1, m = -1, s = -1, ms = -1;
      for (const f of formats) {
        const r = new RegExp("^" + f.regexp + "$");
        results = r.exec(text);
        if (results !== null) {
          h = f.getHour(results);
          if (text.toUpperCase().indexOf("P") > -1 && h < 12) {
            h += 12;
          } else if (text.toUpperCase().indexOf("A") > -1 && h === 12) {
            h = 0;
          }
          m = f.getMinutes(results);
          s = f.getSeconds(results);
          ms = f.getMilliseconds(results);
          break;
        }
      }
      if (results === null) {
        return { valid: false, message: formatError };
      }
      if ((h < 0) || (h > 23) || (m < 0) || (m > 59) || (s < 0) || (s > 59) || (ms < 0) || (ms > 999)) {
        return { valid: false, message: formatError };
      }

      const dt = new Date(0, 0, 0, h, m, s, ms);
      if (
        dt.getHours() !== h || dt.getMinutes() !== m ||
        dt.getSeconds() !== s || dt.getMilliseconds() !== ms
      ) {
        return { valid: false, message: formatError };
      }

      if (bottom) {
        if (dt.getTime() < bottom.getTime()) {
          return { valid: false, message: tooSmallError };
        }
      }

      if (top) {
        if (dt.getTime() > top.getTime()) {
          return { valid: false, message: tooLargeError };
        }
      }

      return { valid: true };
    };
  }
);
