/*
 * Copyright (C) 2011 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER
(1, JavaScriptConstructor, "WRegExpValidator",
   function(mandatory, regexp, modifiers, blankError, invalidError) {

     var r = regexp ? new RegExp("^" + regexp + "$", modifiers) : null;

     this.validate = function(text) {
       if (text.length == 0)
	 if (mandatory)
	   return { valid: false, message: blankError };
	 else
	   return { valid: true };


       if (r)
	 return { valid: r.test(text), message: invalidError };
       else
	 return { valid: true };
     };
   });