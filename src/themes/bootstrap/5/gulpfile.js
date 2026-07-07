/* eslint-env node */
const { series, parallel, src, dest } = require("gulp");
const sass = require("gulp-sass")(require("sass"));
const postcss = require("gulp-postcss");
const autoprefixer = require("autoprefixer");
const cssnano = require("cssnano");
const sourcemaps = require("gulp-sourcemaps");
const through = require("through2");
const rename = require("gulp-rename");
const rtlcss = require("rtlcss");
const applySourceMap = require("vinyl-sourcemaps-apply");

const sassSourceFile = "main.scss";
const outputFolder = "dist";
const outputFile = outputFolder + "/main.css";
const resourceFolder = "../../../../resources/themes/bootstrap/5";

function toRTL() {
  return through.obj(function(file, enc, cb) {
    const result = rtlcss.process(file.contents.toString(), {
      prev: file.sourceMap,
    });

    file.contents = Buffer.from(result);
    if (file.map) {
      applySourceMap(file, result.map);
    }

    this.push(file);
    cb();
  });
}

function compileSass() {
  return src(sassSourceFile)
    .pipe(sourcemaps.init())
    .pipe(sass())
    .pipe(sourcemaps.write("."))
    .pipe(dest(outputFolder));
}
compileSass.description = "Compile, Bootstrap and Wt's .scss files, putting the result in dist/main.css";

function makeLTR() {
  return src(outputFile)
    .pipe(sourcemaps.init())
    .pipe(postcss([autoprefixer, cssnano]))
    .pipe(sourcemaps.write("."))
    .pipe(dest(outputFolder));
}
makeLTR.description = "Autoprefix and minify dist/main.css, overriding it";

function makeRTL() {
  return src(outputFile)
    .pipe(sourcemaps.init())
    .pipe(toRTL())
    .pipe(rename({ suffix: "-rtl" }))
    .pipe(postcss([autoprefixer, cssnano]))
    .pipe(sourcemaps.write("."))
    .pipe(dest(outputFolder));
}
makeRTL.description = "Generate, autoprefix, and minify RTL CSS from dist/main.css";

function copyJs() {
  return src("node_modules/bootstrap/dist/js/bootstrap.bundle.min.js*")
    .pipe(dest(outputFolder));
}
copyJs.description = "Copy over Bootstrap's JavaScript to dist/bootstrap.bundle.min.js";

function copyResources() {
  return src("dist/*")
    .pipe(dest(resourceFolder));
}

const wt = series(parallel(compileSass, copyJs), makeRTL, makeLTR, copyResources);
wt.description = "Compiles CSS and copies the CSS and JavaScript over to Wt's resources folder. " +
  "This task is to be run from Wt's source tree during development of Wt only.";

exports.compileSass = compileSass;
exports.makeRTL = makeRTL;
exports.copyJs = copyJs;
exports.wt = wt;
exports.default = series(parallel(compileSass, copyJs), makeRTL, makeLTR);
