#!/bin/sh
#
# This script uses imagemagick to generate the stripe-XXpx.gif's.
#

for i in `seq 10 40`; do
	convert -size 30x$[2*$i] xc:white \
	    -fill '#f2f2f2' -draw "rectangle 0,$i 30,$[2*$i]" \
	    -fill '#dcdfe8' -draw "line 0,$[$i-1] 30,$[$i-1]" \
	    -fill '#dcdfe8' -draw "line 0,$[2*$i-1] 30,$[2*$i-1]" \
	stripe-${i}px.gif
done
