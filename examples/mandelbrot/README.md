Mandelbrot example
------------------

This example shows renders the famous mandelbrot fractal in small tiles
which are served as individual images. These tiles are calculated on-demand,
as the user navigates through the set.

This is similar to how one navigates through google maps, but in this
case, the images are computed on the fly, instead of being retrieved
from a database.

How to run
----------

See the README in the parent directory.

What it illustrates
-------------------

- the use of a custom `WResource` that computes contents on the fly
- the use of `WRasterImage` for painting raster graphics
- the use of `WVirtualImage` for a virtual image that consists of smaller
  tiled images