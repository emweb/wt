/* eslint-env node */
const gulp = require("gulp");
const rename = require("gulp-rename");
const terser = require("gulp-terser");

const terserOptions = {
  compress: {
    // Turning semicolons into commas breaks Wt's JavaScript as C++ code macro trick
    // It turns two statements like:
    //   WT_DECLARE_WT_MEMBER(...);
    //   WT_DECLARE_WT_MEMBER(...);
    // into:
    //   WT_DECLARE_WT_MEMBER(...),WT_DECLARE_WT_MEMBER(...)
    sequences: false,
  },
  format: {
    comments: "some",
    ecma: "2015",
  },
};

const renameOptions = {
  extname: ".min.js",
};

gulp.task("js", function(done) {
  gulp.src([
    "*.js",
    "!gulpfile.js",
    "!*.min.js",
  ])
    .pipe(terser(terserOptions))
    .pipe(rename(renameOptions))
    .pipe(gulp.dest("."))
    .on("end", done);
});

gulp.task("skeleton", function(done) {
  gulp.src([
    "../web/skeleton/Boot.js",
    "../web/skeleton/Wt.js",
  ])
    .pipe(terser(terserOptions))
    .pipe(rename(renameOptions))
    .pipe(gulp.dest("../web/skeleton"))
    .on("end", done);
});

gulp.task("default", gulp.series("js", "skeleton"));
