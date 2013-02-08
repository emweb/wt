function (editElement, suggestionText, suggestionValue) {
    // editElement is the form element which must be edited.
    // suggestionText is the displayed text for the matched suggestion.
    // suggestionValue is the stored value for the matched suggestion.

    // computed modifiedEditValue and modifiedPos ...

    editElement.value = modifiedEditValue;
    editElement.selectionStart = edit.selectionEnd = modifiedPos;
}
