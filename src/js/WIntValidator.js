/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

/*
 * We could merge this with WDoubleValidator ...
 */

WT_DECLARE_WT_MEMBER
(1, JavaScriptConstructor, "WIntValidator",
   function(mandatory, bottom, top, groupSeparator, blankError,
	    NaNError, tooSmallError, tooLargeError) {
     this.validate = function(text) {
       text = String(text);

       if (text.length == 0)
	 if (mandatory)
	   return { valid: false, message: blankError };
	 else
	   return { valid: true };

       if (groupSeparator != '')
	 text = text.replace(groupSeparator, '', 'g');

       var n = Number(text);

       if (isNaN(n) || (Math.round(n) != n))
	 return { valid: false, message: NaNError };

       if (bottom !== null)
	 if (n < bottom)
	   return { valid: false, message: tooSmallError };
       if (top !== null)
	 if (n > top)
	   return { valid: false, message: tooLargeError };

       return { valid: true };
     };

   });
