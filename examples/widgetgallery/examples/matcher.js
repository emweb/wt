function (editElement) {
    // fetch the location of cursor and current text in the editElement.

    // return a function that matches a given suggestion with the current
    // value of the editElement.
    return function(suggestion) {

	// 1) if suggestion is null, simply return the current text 'value'
	// 2) check suggestion if it matches
	// 3) add highlighting markup to suggestion if necessary

	return { match : ...,      // does the suggestion match ? (boolean)
		 suggestion : ...  // modified suggestion with highlighting
	       };
    }
}
