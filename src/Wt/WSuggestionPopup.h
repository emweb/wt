// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef WSUGGESTION_POPUP_H_
#define WSUGGESTION_POPUP_H_

#include <Wt/WModelIndex.h>
#include <Wt/WPopupWidget.h>
#include <Wt/WJavaScript.h>

namespace Wt {

class WAbstractItemModel;
class WModelIndex;
class WFormWidget;
class WTemplate;

/*! \brief Enumeration that defines a trigger for showing the popup.
 *
 * \sa forEdit()
 */
enum class PopupTrigger {
  /*! \brief Shows popup when the user starts editing.
   *
   * The popup is shown when the currently edited text has a length
   * longer than the \link setFilterLength() filter length\endlink.
   */
  Editing = 0x1,

  /*! \brief Shows popup when user clicks a drop down icon.
   *
   * The lineedit is modified to show a drop down icon, and clicking
   * the icon shows the suggestions, very much like a WComboBox.
   */
  DropDownIcon = 0x2
};

/*! \class WSuggestionPopup Wt/WSuggestionPopup.h Wt/WSuggestionPopup.h
 *  \brief A widget which popups to assist in editing a textarea or lineedit.
 *
 * This widget may be associated with one or more \link WFormWidget
 * WFormWidgets\endlink (typically a WLineEdit or a WTextArea).
 *
 * The popup provides the user with suggestions to enter input. The
 * popup can be used by one or more editors, using forEdit(). The
 * popup will show when the user starts editing the edit field, or
 * when the user opens the suggestions explicitly using a drop down
 * icon or with the down key. The popup positions itself intelligently
 * just below or just on top of the edit field. It offers a list of
 * suggestions that match in some way with the current edit field, and
 * dynamically adjusts this list. The implementation for matching
 * individual suggestions with the current text is provided through a
 * JavaScript function. This function may also highlight part(s) of
 * the suggestions to provide feed-back on how they match.
 *
 * %WSuggestionPopup is an MVC view class, using a simple
 * WStringListModel by default. You can set a custom model using
 * setModel(). The model can provide different text for the suggestion
 * text (Wt::ItemDataRole::Display) and value (editRole()). The member methods
 * clearSuggestions() and addSuggestion() manipulate this model.
 *
 * By default, the popup implements all filtering client-side. To
 * support large datasets, you may enable server-side filtering of
 * suggestions based on the input. The server-side filtering may
 * provide a coarse filtering using a fixed size prefix of the entered
 * text, and complement the client-side filtering. To enable
 * server-side filtering, use setFilterLength() and listen to filter
 * notification using the modelFilter() signal. Whenever a filter
 * event is generated you can adjust the model's content according to
 * the filter (e.g. using a WSortFilterProxyModel). By using
 * setMaximumSize() you can also limit the maximum height of the
 * popup, in which case scrolling is supported (similar to a
 * combo-box).
 *
 * The class is initialized with an Options struct which configures
 * how suggestion filtering and result editing is done. Alternatively,
 * you can provide two JavaScript functions, one for filtering the
 * suggestions, and one for editing the value of the textarea when a
 * suggestion is selected.
 *
 * The matcherJS function must have the following JavaScript signature:
 * 
 * \code
 * function (editElement) {
 *   // fetch the location of cursor and current text in the editElement.
 *
 *   // return a function that matches a given suggestion with the current value of the editElement.
 *   return function(suggestion) {
 *
 *     // 1) if suggestion is null, simply return the current text 'value'
 *     // 2) check suggestion if it matches
 *     // 3) add highlighting markup to suggestion if necessary
 *
 *     return { match : ...,      // does the suggestion match ? (boolean)
 *              suggestion : ...  // modified suggestion with highlighting
 *             };
 *   }
 * }
 * \endcode
 *
 * The replacerJS function that edits the value has the following
 * JavaScript signature.
 *
 * \code
 * function (editElement, suggestionText, suggestionValue) {
 *   // editElement is the form element which must be edited.
 *   // suggestionText is the displayed text for the matched suggestion.
 *   // suggestionValue is the stored value for the matched suggestion.
 *
 *   // computed modifiedEditValue and modifiedPos ...
 *
 *   editElement.value = modifiedEditValue;
 *   editElement.selectionStart = edit.selectionEnd = modifiedPos;
 * }
 * \endcode
 *
 * To style the suggestions, you should style the \<span\> element inside
 * this widget, and the \<span\>."sel" element to style the current selection.
 *
 * Usage example:
 * \if cpp
 * \code
 * // options for email address suggestions
 * Wt::WSuggestionPopup::Options contactOptions
 * = { "<b>",         // highlightBeginTag
 *     "</b>",        // highlightEndTag
 *     ',',           // listSeparator      (for multiple addresses)
 *     " \n",        // whitespace
 *     "-., \"@\n;", // wordSeparators     (within an address)
 *     ", "           // appendReplacedText (prepare next email address)
 *    };
 *
 * Wt::WSuggestionPopup *popup = addWidget(std::make_unique<Wt::WSuggestionPopup>(contactOptions));
 * Wt::WTextArea *textEdit = addWidget(std::make_unique<Wt::WTextArea>());
 * popup->forEdit(textEdit);
 *
 * // load popup data
 * for (unsigned i = 0; i < contacts.size(); ++i)
 *   popup->addSuggestion(contacts[i].formatted(), contacts[i].formatted());
 * \endcode
 * \elseif java
 * \code
 * // options for email address suggestions
 * WSuggestionPopup.Options contactOptions = new WSuggestionPopup.Options(); 
 * contactOptions.highlightBeginTag = "<b>";
 * contactOptions.highlightEndTag = "</b>";
 * contactOptions.listSeparator = ','; //for multiple addresses)
 * contactOptions.whitespace = " \n";
 * contactOptions.wordSeparators = "-., \"@\n;"; //within an address
 * contactOptions.appendReplacedText = ", "; //prepare next email address
 *	
 * WSuggestionPopup popup = new WSuggestionPopup(contactOptions, this);
 * 
 * WTextArea textEdit = new WTextArea(this);
 * popup.forEdit(textEdit);
 *		 
 * // load popup data
 * for (int i = 0; i < contacts.size(); ++i)
 * popup.addSuggestion(contacts.get(i).formatted(), contacts.get(i).formatted());
 * \endcode
 * \endif
 *
 * A screenshot of this example:
 * <TABLE border="0" align="center"> <TR> <TD> 
 * \image html WSuggestionPopup-default-1.png "An example WSuggestionPopup (default)"
 * </TD> <TD>
 * \image html WSuggestionPopup-polished-1.png "An example WSuggestionPopup (polished)"
 * </TD> </TR> </TABLE>
 *
 * When using the DropDownIcon trigger, an additional style class is
 * provided for the edit field: <tt>Wt-suggest-dropdown</tt>, which
 * renders the icon to the right inside the edit field. This class may
 * be used to customize how the drop down icon is rendered.
 *
 * \note This widget requires JavaScript support.
 *
 * \ingroup modelview
 */
class WT_API WSuggestionPopup : public WPopupWidget
{
public:
  /*! \brief Typedef for enum Wt::PopupTrigger */
  typedef PopupTrigger Trigger;

  /*! \brief A configuration object to generate a matcher and replacer
   *         JavaScript function.
   *
   * \sa WSuggestionPopup
   */
  struct Options {
#ifdef WT_TARGET_JAVA
    /*! \brief Constructor
     */
    Options();
#endif

    /*! \brief Open tag to highlight a match in a suggestion.
     *
     * Must be an opening markup tag, such as &lt;b&gt;.
     *
     * Used during matching.
     */
    std::string highlightBeginTag;

    /*! \brief Close tag to highlight a match in a suggestion.
     *
     * Must be a closing markup tag, such as &lt;/b&gt;.
     *
     * Used during matching.
     */
    std::string highlightEndTag;

    /*! \brief When editing a list of values, the separator used
     *         for different items.
     *
     * For example, ',' to separate different values on comma. Specify
     * 0 ('\\0') for no list separation.
     *
     * Used during matching and replacing.
     */
    char listSeparator;

    /*! \brief When editing a value, the whitespace characters ignored
     *         before the current value.
     *
     * For example, " \n" to ignore spaces and newlines.
     *
     * Used during matching and replacing.
     */
    std::string whitespace;

    /*! \brief Characters that start a word in a suggestion to match against.
     *
     * For example, " .@" will also match with suggestion text after a space,
     * a dot (.) or an at-symbol (@). Alternatively you may also specify this
     * as a regular expression in \p wordStartRegexp.
     *
     * Used during matching.
     */
    std::string wordSeparators;

    /*! \brief When replacing the current edited value with suggestion value,
     *         append the following string as well.
     *
     * Used during replacing.
     */
    std::string appendReplacedText;

    /*! \brief Regular expression that starts a word in a suggestion to
     *         match against.
     *
     * When empty, the value of \p wordSeparators is used instead.
     *
     * Used during replacing.
     */
    std::string wordStartRegexp;
  };

  /*! \brief Creates a suggestion popup.
   *
   * The popup using a standard matcher and replacer implementation
   * that is configured using the provided \p options.
   *
   * \sa generateMatcherJS(), generateReplacerJS()
   */
  WSuggestionPopup(const Options& options);

  /*! \brief Creates a suggestion popup with given matcherJS and replacerJS.
   *
   * See supra for the expected signature of the matcher and replace
   * JavaScript functions.
   */
  WSuggestionPopup(const std::string& matcherJS, const std::string& replacerJS);

  /*! \brief Lets this suggestion popup assist in editing an edit field.
   *
   * A single suggestion popup may assist in several edits by repeated calls
   * of this method.
   *
   * The \p popupTriggers control how editing is triggered (either by the user
   * editing the field by entering keys or by an explicit drop down menu that
   * is shown inside the edit).
   *
   * \sa removeEdit()
   */
  void forEdit(WFormWidget *edit,
	       WFlags<PopupTrigger> popupTriggers = PopupTrigger::Editing);

  /*! \brief Removes the edit field from the list of assisted editors.
   *
   * The editor will no longer be assisted by this popup widget.
   *
   * \sa forEdit().
   */
  void removeEdit(WFormWidget *edit);

  /*! \brief Shows the suggestion popup at an edit field.
   *
   * This is equivalent to the user triggering the suggestion popup to
   * be shown.
   */
  void showAt(WFormWidget *edit);

  /*! \brief Clears the list of suggestions.
   *
   * This clears the underlying model.
   *
   * \sa addSuggestion()
   */
  void clearSuggestions();

  /*! \brief Adds a new suggestion.
   *
   * This adds an entry to the underlying model. The \p suggestionText
   * is set as Wt::ItemDataRole::Display and the \p suggestionValue (which is
   * inserted into the edit field on selection) is set as editRole().
   *
   * \sa clearSuggestions(), setModel()
   */
  void addSuggestion(const WString& suggestionText,
		     const WString& suggestionValue = WString::Empty);

  /*! \brief Sets the model to be used for the suggestions.
   *
   * The \p model may not be \c 0.
   *
   * The default value is a WStringListModel that is owned by the
   * suggestion popup.
   *
   * The Wt::ItemDataRole::Display is used for the suggestion text. The
   * editRole() is used for the suggestion value, unless empty, in
   * which case the suggestion text is used as value.
   *
   * \sa setModelColumn(int)
   */
  void setModel(const std::shared_ptr<WAbstractItemModel>& model);

  /*! \brief Returns the data model.
   *
   * \sa setModel()
   */
  std::shared_ptr<WAbstractItemModel> model() const { return model_; }

  /*! \brief Sets the column in the model to be used for the items.
   *
   * The column \p index in the model will be used to retrieve data.
   *
   * The default value is 0.
   *
   * \sa setModel()
   */
  void setModelColumn(int index);

  /*! \brief Sets a default selected value.
   *
   * \p row is the model row that is selected by default (only if it
   * matches the current input).
   *
   * The default value is -1, indicating no default.
   */
  void setDefaultIndex(int row);

  /*! \brief Returns the default value.
   *
   * \sa setDefaultValue()
   */
  int defaultIndex() const { return defaultValue_; }

  /*! \brief Creates a standard matcher JavaScript function.
   *
   * This returns a JavaScript function that provides a standard
   * implementation for the matching input, based on the given \p
   * options.
   */
  static std::string generateMatcherJS(const Options& options);

  /*! \brief Creates a standard replacer JavaScript function.
   *
   * This returns a JavaScript function that provides a standard
   * implementation for reacting to a match activation, editing the
   * line edit text.
   */
  static std::string generateReplacerJS(const Options& options);

  /*! \brief Sets the minimum input length before showing the popup.
   *
   * When the user has typed this much characters, filterModel() is
   * emitted which allows you to filter the model based on the initial
   * input. The filtering is done as long as the model indicates that
   * results are partial by setting a ItemDataRole::StyleClass of "Wt-more-data"
   * on the last item.
   *
   * The default value is 0.
   *
   * A value of -1 is a equivalent to 0 but filtering is always applied
   * as if the last item always has "Wt-more-data" (for backwards
   * compatibility)
   *
   * \sa filterModel()
   */
  void setFilterLength(int count);

  /*! \brief Returns the filter length.
   *
   * \sa setFilterLength()
   */
  int filterLength() const { return filterLength_; }

  /*! \brief %Signal that indicates that the model should be filtered.
   *
   * The argument is the initial input. When \link
   * PopupTrigger::Editing Editing\endlink is used as edit
   * trigger, its length will always equal the filterLength(). When
   * \link PopupTrigger::DropDownIcon DropDownIcon\endlink is used
   * as edit trigger, the input length may be less than
   * filterLength(), and the the signal will be called repeatedly as
   * the user provides more input.
   *
   * For example, if you are using a WSortFilterProxyModel, you could
   * react to this signal with:
   * \if cpp
   * \code
   * void MyClass::filterSuggestions(const WString& filter)
   * {
   *   proxyModel->setFilterRegExp(filter + ".*");
   * }
   * \endcode
   * \elseif java
   * \code
   * public filterSuggestions(String filter) {
   *   proxyModel.setFilterRegExp(filter + ".*");
   * }
   * \endcode
   * \endif 
   */
  Signal<WT_USTRING>& filterModel() { return filterModel_; }

  /*! \brief %Signal emitted when a suggestion was selected.
   *
   * The selected item is passed as the first argument and the editor as
   * the second.
   */
  Signal<int, WFormWidget *>& activated() { return activated_; }

  /*! \brief When drop down icon is clicked the popup content will be
   *   unfiltered.
   *  \sa forEdit() and enum PopupTrigger
   */
  void setDropDownIconUnfiltered(bool isUnfiltered);

  /*! \brief Returns the last activated index
   * 
   * Returns -1 if the popup hasn't been activated yet.
   *
   * \sa activated()
   */
  int currentItem() const { return currentItem_; }

  /*! \brief Sets the role used for editing the line edit with a chosen item.
   *
   * The default value is ItemDataRole::User.
   */
  void setEditRole(ItemDataRole role) { editRole_ = role; }

  /*! Returns the role used for editing the line edit.
   *
   * \a setEditRole()
   */
  ItemDataRole editRole() const { return editRole_; }

private:
  WContainerWidget *impl_;
  std::shared_ptr<WAbstractItemModel> model_;
  int modelColumn_;
  int filterLength_;
  bool filtering_;
  int defaultValue_;
  bool isDropDownIconUnfiltered_;
  int currentItem_;
  ItemDataRole editRole_;

  std::string       matcherJS_;
  std::string       replacerJS_;

  Signal<WT_USTRING> filterModel_;
  Signal<int, WFormWidget *> activated_;

  std::vector<Wt::Signals::connection> modelConnections_;

  std::string currentInputText_;

  JSignal<std::string> filter_;
  JSignal<std::string, std::string> jactivated_;

  std::vector<WFormWidget *> edits_;

  void init();
  void scheduleFilter(std::string input);
  void doFilter(std::string input);
  void doActivate(std::string itemId, std::string editId);
  void connectObjJS(EventSignalBase& s, const std::string& methodName);

  void modelRowsInserted(const WModelIndex& parent, int start, int end);
  void modelRowsRemoved(const WModelIndex& parent, int start, int end);
  void modelDataChanged(const WModelIndex& topLeft,
			const WModelIndex& bottomRight);
  void modelLayoutChanged();
  bool partialResults() const;

  void defineJavaScript();

protected:
  virtual void render(WFlags<RenderFlag> flags) override;
};

W_DECLARE_OPERATORS_FOR_FLAGS(PopupTrigger)

}

#endif // WSUGGESTION_POPUP_H_
