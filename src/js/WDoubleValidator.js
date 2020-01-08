/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

/*
 * We could merge this with WIntValidator ...
 */

WT_DECLARE_WT_MEMBER
(1, JavaScriptConstructor, "WDoubleValidator",
   function(mandatory, ignoreTrailingSpaces, bottom, top, decimalPoint, groupSeparator, blankError,
	    NaNError, tooSmallError, tooLargeError) {
     this.validate = function(text) {
       text = String(text);
	   
	   if (ignoreTrailingSpaces) 
		 text = text.trim();

       function toRegexp(value) {
	  return value.replace(new RegExp("([\\^\\\\\\][\\-.$*+?()|{}])","g"), "\\$1");
       }

       if (text.length == 0)
	 if (mandatory)
	   return { valid: false, message: blankError };
	 else
	   return { valid: true };

       if (groupSeparator != '')
	 text = text.replace(new RegExp(toRegexp(groupSeparator), 'g'), '');
       if (decimalPoint != '.')
	 text = text.replace(decimalPoint, '.');

	   if(text.indexOf(' ') >= 0 && !ignoreTrailingSpaces)
		 return { valid: false, message: NaNError };

       var n = Number(text);

       if (isNaN(n))
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
