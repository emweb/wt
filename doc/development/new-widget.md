# Adding a new widget

The document details some guidelines to consider when adding a new widget to Wt.

## Choice of base class

There are two derived classes of `WWidget`:

- `WWebWidget`
- `WCompositeWidget`

### WWebWidget

`WWebWidget` usually corresponds with a specific HTML tag, or with a type of `<input>`.
From the `WWebWidget` documentation:

> A base class for widgets with an HTML counterpart.
>
> All descendants of WWebWidget implement a widget which corresponds almost one-on-one with an HTML element.
> These widgets provide most capabilities of these HTML elements, but rarely make no attempt to do anything more.

This means you should generally make your widget a `WWebWidget` if your widget clearly
corresponds to a specific HTML element. Prefer using `WCompositeWidget` if that is not clear.

Note: most `WWebWidget`s are `WInteractWidget`s as well. If your widget responds to user events
it should probably be a `WInteractWidget`. Many `WInteractWidget`s are also `WFormWidget`s:
it has a value, can have a label associated with it, has a `change` event, etc. These are
typically different kinds of `<input>` elements, `<select>`, or `<textarea>`.

### WCompositeWidget

As a general rule, use `WCompositeWidget` whenever `WWebWidget` does not apply.
From the `WCompositeWidget` documentation:

> A widget that hides the implementation of composite widgets.
>
> ...
>
> Using this class you can completely hide the implementation of your composite widget, and provide access to
> only the standard WWidget methods.

Composite widgets are implemented using another widget, often a `WContainerWidget` or
`WTemplate`, often containing more widgets. The widget that is used for the implementation
is hidden from the public API, though, because it may change in the future, or because
providing direct access to the implementation may lead to breakage.

For example, as of Wt 4.10.0, `WTabWidget` is implemented using a `WContainerWidget` that
contains a `WMenu` and a `WStackedWidget`. In a later version of Wt, however, we may decide
that using a `WTemplate` is more appropriate. Also, if we allow access directly to the
underlying `WContainerWidget`, then user code may break the `WTabWidget`
in all sorts of unpredictable ways if the user treats it just like any other `WContainerWidget`.
The implementation of `WTabWidget` may assume and rely on the fact that the `WContainerWidget`
contains exactly two widgets, and adding another widget to it may break things in unpredictable ways.

Certain `WCompositeWidget`s are also implemented using an external JavaScript library.
A `WGoogleMap` or `WLeafletMap` can't be treated just like a normal `WContainerWidget`
or `WTemplate`, because there's JavaScript code attached to them that will modify the
contents in all sorts of ways that Wt doesn't know about.

## Cross-cutting features

Wt has a number of features that you'll need to keep in the back of your head when
working on existing widgets or implementing new widgets. These features need a bit
more attention and testing to make sure they continue to work.

### Plain HTML mode

When JavaScript support is not available[^1], Wt presents a plain HTML version to
the browser. However, this means that sometimes we have to serve slightly different
content depending on whether JavaScript is available.

If your widget uses JavaScript, you can either:

- Limit the widget to being JavaScript-only. This is typically the
  case when the widget wraps some JavaScript library, like `WLeafletMap`.
- Provide an alternative experience when JavaScript is not
  available, e.g. `WTableView` provides "previous" and "next"
  buttons instead of on demand loading. You can check
  `WEnvironment::ajax()` to see whether JavaScript is available.
- Make sure a core set of features still works when JavaScript is enabled.
  Most event signals except for `clicked()` are not emitted when JavaScript
  is not enabled.

### Progressive bootstrap

When progressive bootstrap is enabled, a non JavaScript enabled
version is rendered first before a JavaScript enabled version is
rendered. Every widget in the widget tree will receive an
`enableAjax()` call.

This means that on first pass, `WEnironment::ajax()` will return `false`.

You should make sure that regardless of if progressive bootstrap is enabled, the
end result is the same.

Example issues:

- [Native WSlider not working on progressive bootstrap](https://redmine.webtoolkit.eu/issues/7856)
  (still unresolved as of Wt 4.10.0)
- [Interactive charts does not work with progressive bootstrap enabled](https://redmine.webtoolkit.eu/issues/7514)
  (still unresolved as of Wt 4.10.0)

### Full widget rerender

For some reason, your widget may need to be rerendered entirely.
This could be because:

- `<reload-is-new-session>` is set to `false` and the user refreshes
  the page
- The session is resumed after being suspended 
  (`WApplication::suspend()`)
- The widget is removed and later re-added to the page
- JavaScript support is unavailable, so the entire page
  has to be rendered on every request.

Make sure that when this happens, the widget is rerendered properly.

For example, if you're wrapping a JavaScript library, you may be
tempted to have your setters call only `doJavaScript()`
to change some properties. `WLeafletMap::panTo()` could simply execute
the necessary JavaScript to pan the map to a different position. However,
when you then rerender the map, the position that you previously panned to
would be lost.

Tip: `setJavaScriptMember` is always reexecuted when the widget
is rerendered, whereas any change you do with `doJavaScript()`
is not preserved.

## Foot notes

[^1]: JavaScript support may be disabled in the browser,
      or the user agent may not support it, e.g. web crawlers
      or terminal browsers like
      [`lynx`](https://lynx.invisible-island.net/).
