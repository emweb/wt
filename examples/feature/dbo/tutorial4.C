/*
 * Copyright (C) 2010 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/*****
 * This file is part of the Wt::Dbo tutorial:
 * http://www.webtoolkit.eu/wt/doc/tutorial/dbo/tutorial.html
 *****/

/*****
 * Dbo tutorial section 7.2
 *  Changing or disabling the "version" field
 *****/

#include <Wt/Dbo/Dbo.h>

class Post;

namespace Wt {
  namespace Dbo {

    template<>
    struct dbo_traits<Post> : public dbo_default_traits {
      static const char *versionField() {
        return 0;
      }
    };

  }
}

#include "tutorial2.C"

