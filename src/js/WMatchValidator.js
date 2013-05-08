WT_DECLARE_WT_MEMBER(1, JavaScriptConstructor, "WMatchValidator",
 function(elem, mandatory, blankError, invalidDataError, mismatchError) {
	this.validate = function(text) {
		var v;

		if((typeof(elem) === "undefined") || (elem === null)) {
			return {valid : false, message : invalidDataError};
		}

		if(elem.options) {
			v = elem.options.item(elem.selectedIndex).text;
		} else {
			v = elem.value;
		};

		if(text != v) {
			return {valid : false, message : mismatchError};
		} if((text.length == 0) && (mandatory)) {
			return { valid: false, message: blankError };
		} else {
			return {valid : true};
		}
	}
 }
);

