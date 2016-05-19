DEST=../approot/src.xml

echo "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>" > $DEST
echo "<messages>" >> $DEST

for i in *.cpp; do
  echo $i;
  echo "  <message id=\"src-`basename $i .cpp`\">" >> $DEST
  cat $i | awk '
BEGIN { p = 1; ifndef = 0 }
/^#ifndef WT_TARGET_JAVA/ { ifndef = 1; next }
/^#else/ { if (ifndef) { p = 0; next } }
/^#endif/ { if (ifndef) { p = 1; ifndef = 0; next } }
{ if (p) { print } }' | sed -e 's/extern //g' | grep -v SAMPLE_ | grep -v 'ifdef' | grep -v 'ifndef' | grep -v HPDF_MAJOR | grep -v 'endif' | pygmentize -l cpp -f html >> $DEST
  echo "  </message>" >> $DEST
done

for i in *.js; do
  echo $i;
  echo "  <message id=\"src-`basename $i .js`\">" >> $DEST
  cat $i | grep -v SAMPLE_ | pygmentize -l js -f html >> $DEST
  echo "  </message>" >> $DEST
done

for i in ../docroot/style/*.css; do
  echo $i;
  echo "  <message id=\"src-`basename $i .css`\">" >> $DEST
  cat $i | grep -v SAMPLE_ | pygmentize -l css -f html >> $DEST
  echo "  </message>" >> $DEST
done

echo "</messages>" >> $DEST
