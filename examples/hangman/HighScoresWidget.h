/* this is a -*-C++-*- file
 * Copyright (C) 2005 Wim Dumon
 *
 * See the LICENSE file for terms of use.
 */

#include <Wt/WContainerWidget>

using namespace Wt;

class HighScoresWidget: public WContainerWidget
{
   public:
      HighScoresWidget(const std::wstring &user,
		       WContainerWidget *parent = 0);
      void update();

   private:
      std::wstring User;
};
