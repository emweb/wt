Painting example
----------------

This example illustrates the use of Wt's painting API to render vector
graphics in the browser.

The emweb logo is rendered using SVG, VML or Canvas depending on
browser capabilities. If the browser is standards compliant, then SVG
is used for positive angles and Canvas for negative angles of the logo
orientation.

The sliders that control the rendering are actually also, in part,
rendered using a painted widget, for the location of the thicks.

How to run
----------

See the README in the parent directory.

What it illustrates
-------------------

- how to implement a custom `WPaintedWidget`
- how to use `WSlider`