# JavaScript

Wt uses a combination of generated JavaScript and static JavaScript files.
The minified version of those files is embedded in the source code at
compile time.

These JavaScript files can be found in [`src/js`](../../src/js)
and [`src/web/skeleton`](../../src/web/skeleton).

## Minified JavaScript

We check the minified version into git, so it's not necessary to minify
them when building Wt.

They do have to be re-minified when they're changed though. We use
[gulp-terser](https://www.npmjs.com/package/gulp-terser) for this.
You will need to have `npm` installed on your system:


```shell
cd src/js
npm install
npm run minify
```

This will minify all of the JavaScript in `src/js` and `src/web/skeleton`.

## Debugging JavaScript

Configure Wt with `-DDEBUG_JS=ON` to make Wt serve the non-minified JavaScript
files in `src/js` from the filesystem instead of embedded the minified version.
This is useful for debugging during development. Note that this hard-codes the
path to the JavaScript files in of the source tree, so Wt binaries built with
this option can't be installed onto another system.
