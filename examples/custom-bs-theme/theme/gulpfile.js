/* eslint-env node */
const { series, src, dest } = require("gulp");
const sass = require("gulp-sass")(require("sass"));
const postcss = require("gulp-postcss");
const autoprefixer = require("autoprefixer");
const cssnano = require("cssnano");
const sourcemaps = require("gulp-sourcemaps");

const outputFolder = "../docroot/css";

function compileSass() {
  return src("theme.scss")
    .pipe(sourcemaps.init())
    .pipe(sass({
      includePaths: [
        "../../../src", // Inside Wt source tree
        "../../../../share/Wt", // If installed on Windows
        "../../../../../share/Wt", // If installed otherwise
      ],
    }))
    .pipe(postcss([autoprefixer, cssnano]))
    .pipe(sourcemaps.write("."))
    .pipe(dest(outputFolder));
}

exports.compileSass = compileSass;
exports.default = series(compileSass);
