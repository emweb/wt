/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER
  (10, "ChildrenResize",
    function(self, w, h) {
      var j, jl, c, WT = this;
      self.style.height = h + 'px';

      if (WT.boxSizing(self)) {
	h -= WT.px(self, 'marginTop');
        h -= WT.px(self, 'marginBottom');
        h -= WT.px(self, 'borderTopWidth');
        h -= WT.px(self, 'borderBottomWidth');
        h -= WT.px(self, 'paddingTop');
        h -= WT.px(self, 'paddingBottom');

	w -= WT.px(self, 'marginLeft');
        w -= WT.px(self, 'marginRight');
        w -= WT.px(self, 'borderLeftWidth');
        w -= WT.px(self, 'borderRightWidth');
        w -= WT.px(self, 'paddingLeft');
        w -= WT.px(self, 'paddingRight');
      }

      var cheight = h + 'px';

      for (j=0, jl=self.childNodes.length; j < jl; ++j) {
	c=self.childNodes[j];
	if (c.nodeType == 1) {
	  if (c.wtResize)
	    c.wtResize(c, w, h);
          else
            if (c.style.height != cheight)
              c.style.height = cheight;
	}
      }
    }
  );
