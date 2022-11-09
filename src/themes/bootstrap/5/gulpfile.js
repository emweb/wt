/* eslint-env node */
const { series, parallel, src, dest } = require("gulp");
const sass = require("gulp-sass")(require("sass"));
const postcss = require("gulp-postcss");
const autoprefixer = require("autoprefixer");
const cssnano = require("cssnano");
const sourcemaps = require("gulp-sourcemaps");

const sassSourceFile = "main.scss";
const outputFolder = "dist";
const resourceFolder = "../../../../resources/themes/bootstrap/5";

function compileSass() {
  return src(sassSourceFile)
    .pipe(sourcemaps.init())
    .pipe(sass())
    .pipe(postcss([autoprefixer, cssnano]))
    .pipe(sourcemaps.write("."))
    .pipe(dest(outputFolder));
}
compileSass.description =
  "Compile, autoprefix, and minify Bootstrap and Wt's .scss files, putting the result in dist/main.css";

function copyJs() {
  return src("node_modules/bootstrap/dist/js/bootstrap.bundle.min.js*")
    .pipe(dest(outputFolder));
}
copyJs.description = "Copy over Bootstrap's JavaScript to dist/bootstrap.bundle.min.js";

function copyResources() {
  return src("dist/*")
    .pipe(dest(resourceFolder));
}

const wt = series(parallel(compileSass, copyJs), copyResources);
wt.description = "Compiles CSS and copies the CSS and JavaScript over to Wt's resources folder. " +
  "This task is to be run from Wt's source tree during development of Wt only.";

exports.compileSass = compileSass;
exports.copyJs = copyJs;
exports.wt = wt;
exports.default = parallel(compileSass, copyJs);
