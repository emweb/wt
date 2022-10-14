# JavaScript

Wt uses a combination of generated JavaScript and static JavaScript files.
The minified version of those files is embedded in the source code at
compile time.

These JavaScript files can be found in [`src/js`](../../src/js)
and [`src/web/skeleton`](../../src/web/skeleton).

## Tooling

We use various tools, all of which are installed using [pnpm](https://pnpm.io).

You'll need a recent version of [Node.js](https://nodejs.org)
to install some dependencies, like [dprint](https://dprint.dev).
You can use `pnpm` to install a more recent version (e.g. Ubuntu
22.04 comes with version 12, which is too old):

```shell
pnpm env use --global lts
```

In `src/js` you can install and run all the tools:

```shell
cd src/js
pnpm install
```

## Formatting

We use [dprint](https://dprint.dev) for formatting. You can check formatting
and format all files in `src/js`:

```shell
cd src/js
pnpm run checkfmt # Check formatting
pnpm run fmt # Run formatter
```

If you install the dprint extension for Visual Studio Code, then your
could should automatically format on save.

## Minified JavaScript

We check the minified version into git, so it's not necessary to minify
them when building Wt.

They do have to be re-minified when they're changed though. We use
[gulp-terser](https://www.npmjs.com/package/gulp-terser) for this:

```shell
cd src/js
pnpm run minify
```

This will minify all JavaScript in `src/js` and `src/web/skeleton`.

## Linting

We use [ESLint](https://eslint.org) for linting. If you use Visual Studio Code
to edit the JavaScript, you can use the ESLint extension. You can also run ESLint
from the command line:

```shell
cd src/js
pnpm run lint
```

This will run the linter on all JavaScript in `src/js` and `src/web/skeleton`.

## Debugging JavaScript

Configure Wt with `-DDEBUG_JS=ON` to make Wt serve the non-minified JavaScript
files in `src/js` from the filesystem instead of embedded the minified version.
This is useful for debugging during development. Note that this hard-codes the
path to the JavaScript files in of the source tree, so Wt binaries built with
this option can't be installed onto another system.
