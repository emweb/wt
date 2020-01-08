/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER
(1, JavaScriptConstructor, "WDateValidator",
   function(mandatory, formats, bottom, top, blankError,
	    formatError, tooSmallError, tooLargeError) {

   this.validate = function(text) {
     if (text.length == 0)
       if (mandatory)
	   return { valid: false, message: blankError };
	 else
	   return { valid: true };

     var results;

     var m = -1, d = -1, y = -1;

     for (var i = 0, il = formats.length; i < il; ++i) {
       var f = formats[i];
       var r = new RegExp("^" + f.regexp + "$");

       results = r.exec(text);
       if (results != null) {
	 m = f.getMonth(results);
	 d = f.getDay(results);
	 y = f.getYear(results);

	 break;
       }
     }

     if (results == null)
       return { valid: false, message: formatError };

     if ((d <= 0) || (d > 31) || (m <=0 ) || (m > 12))
       return { valid: false, message: formatError };

     var dt = new Date(y, m - 1, d);

     if (dt.getDate() != d ||
         dt.getMonth() != m-1 ||
         dt.getFullYear() != y ||
	 dt.getFullYear() < 1400) {
       return { valid: false, message: formatError };
     }

     if (bottom)
       if (dt.getTime() < bottom.getTime())
	 return { valid: false, message: tooSmallError};

     if (top)
       if (dt.getTime() > top.getTime())
	 return { valid:false, message: tooLargeError};


     return { valid: true };
   };

   });
