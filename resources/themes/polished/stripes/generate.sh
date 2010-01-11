#
# This script uses imagemagick to generate from stripe-template.gif, which
# is the same as stripe-30px.gif, the other versions (10px to 29px)
#
# stripe-XXpx.gif is a template of height 2x XX px, suitable for a table view
# with line height XX px.
#

cp stripe-template.gif stripe-30px.gif
for i in `seq 10 29`; do
	convert stripe-template.gif -crop 30x$[2*$i]+0+$[30-$i]! stripe-${i}px.gif
done
