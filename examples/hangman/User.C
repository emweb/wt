#include "User.h"

using namespace Wt;
using namespace Wt::Dbo;

User::User(const std::string &name, const std::string &password) :
  name(name),
  password(password),
  gamesPlayed(0),
  score(0)
{

}
