DEST=../approot/src.xml

echo "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>" > $DEST
echo "<messages>" >> $DEST

for i in *.cpp; do
  echo $i;
  echo "  <message id=\"src-`basename $i .cpp`\">" >> $DEST
  cat $i | sed -e 's/extern //g' | grep -v SAMPLE_ | grep -v 'ifdef' | grep -v HPDF_MAJOR | grep -v 'endif' | pygmentize -l cpp -f html >> $DEST
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
