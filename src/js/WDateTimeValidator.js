/*
 * Copyright (C) 2021 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER(
  1,
  JavaScriptConstructor,
  "WDateTimeValidator",
  function(mandatory, formats, bottom, top, blankError, formatError, tooSmallError, tooLargeError) {
    this.validate = function(text) {
      if (text.length === 0) {
        if (mandatory) {
          return { valid: false, message: blankError };
        } else {
          return { valid: true };
        }
      }

      let results;

      let mo = -1, d = -1, y = -1, h = -1, mi = -1, s = -1, ms = -1;

      for (const f of formats) {
        const r = new RegExp("^" + f.regexp + "$");

        results = r.exec(text);
        if (results !== null) {
          mo = f.getMonth(results);
          d = f.getDay(results);
          y = f.getYear(results);
          h = f.getHour(results);
          if (text.toUpperCase().indexOf("P") > -1 && h < 12) {
            h += 12;
          } else if (text.toUpperCase().indexOf("A") > -1 && h === 12) {
            h = 0;
          }
          mi = f.getMinutes(results);
          s = f.getSeconds(results);
          ms = f.getMilliseconds(results);
          break;
        }
      }

      if (results === null) {
        return { valid: false, message: formatError };
      }

      if (
        (d <= 0) || (d > 31) || (mo <= 0) || (mo > 12) ||
        (h < 0) || (h > 23) || (mi < 0) || (mi > 59) ||
        (s < 0) || (s > 59) || (ms < 0) || (ms > 999)
      ) {
        return { valid: false, message: formatError };
      }

      const dt = new Date(y, mo - 1, d, h, mi, s, ms);

      if (
        dt.getDate() !== d ||
        dt.getMonth() !== mo - 1 ||
        dt.getFullYear() !== y ||
        dt.getFullYear() < 1400 ||
        dt.getHours() !== h ||
        dt.getMinutes() !== mi ||
        dt.getSeconds() !== s ||
        dt.getMilliseconds() !== ms
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
