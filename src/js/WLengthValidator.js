/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER
(1, JavaScriptConstructor, "WLengthValidator",
   function(mandatory, minLength, maxLength, blankError,
	    tooShortError, tooLongError) {
     this.validate = function(text) {
       if (text.length == 0)
	 if (mandatory)
	   return { valid: false, message: blankError };
	 else
	   return { valid: true };

       if (minLength !== null)
	 if (text.length < minLength)
	   return { valid: false, message: tooShortError };

       if (maxLength !== null)
	 if (text.length > maxLength)
	   return { valid: false, message: tooLongError };

       return { valid: true };
     };

   });