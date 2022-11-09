# Custom Bootstrap theme example

This is a simple application with a button and a customized Bootstrap 5 theme applied.

- The `theme` directory contains the sources for the custom theme.
  If you customize `theme.scss`, then you can rebuild it with:
  ```shell
  npm install
  npm run build
  ```
  Note that you will have to change the path to the `wt` directory if you're building this example
  outside the Wt source tree.
- `Theme.h` and `Theme.C` contain the C++ sources of the customized theme, overriding the stylesheets
  to load our custom theme.

## How to run

See the README in the parent directory, but add the `--docroot=docroot` argument.

## What it illustrates

- `WBootstrap5Theme`, and how it can be customized
- Using NPM to build a custom Bootstrap 5 based Wt theme, with:
  - Sass compilation
  - Auto-prefixing
  - Minification
  - Source maps
