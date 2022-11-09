# Themes

The Bootstrap 5 and widget gallery themes are built from source,
with the compiled result checked into git, so they have to be rebuilt
when they're changed.

Just like for the JavaScript, we use PNPM, more info about that in the
[javascript.md](./javascript.md) file.

## Building the Bootstrap 5 theme

```shell
cd src/themes/bootstrap/5
pnpm install # Install node_modules
pnpm run build-wt # This first runs the build step (pnpm run build),
                  # and then copies the result to the correct resources folders
```

## Building the widgetgallery theme

```shell
cd examples/widgetgallery/style
pnpm install # Install node_modules
pnpm run build
```
