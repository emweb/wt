/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER
  (10, "ChildrenResize",
    function(self, w, h) {
      var j,jl,c;
      self.style.height = h + 'px';
      for (j=0, jl=self.childNodes.length; j < jl; ++j) {
	c=self.childNodes[j];
	if (c.nodeType == 1) {
	  if (c.wtResize)
	    c.wtResize(c, w, h);
          else
            if (c.style.height != self.style.height)
              c.style.height = self.style.height;
	}
      }
    }
  );
