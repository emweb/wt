/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER
(1, JavaScriptConstructor, "WRegExpValidator",
   function(mandatory, regexp, modifiers, blankError, invalidError) {

     var r = regexp ? new RegExp(regexp, modifiers) : null;

     this.validate = function(text) {
       if (text.length == 0)
	 if (mandatory)
	   return { valid: false, message: blankError };
	 else
	   return { valid: true };

       if (r) {
           var result = r.exec(text);
           if (result !== null && result[0].length === text.length)
               return { valid: true };
           else
               return { valid: false, message: invalidError };
       } else
           return { valid: true };
     };
   });
