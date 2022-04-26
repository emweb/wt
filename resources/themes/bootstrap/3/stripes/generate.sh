#!/bin/bash
#
# This script uses imagemagick to generate the stripe-XXpx.gif's.
#

for i in `seq 10 40`; do
  convert -size 30x$[2*$i] xc:white \
          -fill '#f9f9f9' -draw "rectangle 0,0 30,$i" \
  stripe-${i}px.gif
done
