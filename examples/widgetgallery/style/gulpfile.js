/* eslint-env node */
const { series, src, dest } = require("gulp");
const sass = require("gulp-sass")(require("sass"));
const postcss = require("gulp-postcss");
const autoprefixer = require("autoprefixer");
const cssnano = require("cssnano");
const sourcemaps = require("gulp-sourcemaps");

const sassSourceFiles = ["wt.scss", "jwt.scss"];
const outputFolder = "../docroot/style";

function compileSass() {
  return src(sassSourceFiles)
    .pipe(sourcemaps.init())
    .pipe(sass())
    .pipe(postcss([autoprefixer, cssnano]))
    .pipe(sourcemaps.write("."))
    .pipe(dest(outputFolder));
}

exports.compileSass = compileSass;
exports.default = series(compileSass);
