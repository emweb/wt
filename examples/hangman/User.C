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

int User::findRanking(Session& session) const
{
 Transaction transaction(session);

 int ranking  = 
   session.query<int>("select distinct count(score) from user")
   .where("score > ?")
   .bind(score);

  transaction.commit(); 
 
  return ranking + 1;
}
