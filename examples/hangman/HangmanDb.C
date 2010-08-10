#include <iostream>
#include <iomanip>
#include <string>
#include <fstream>

#include <mysql++/mysql++.h>
#include <Wt/WStringUtil>

#include <Wt/WApplication>
#include "HangmanDb.h"

using namespace mysqlpp;

std::string HangmanDb::DbUser()
{
	std::string retval;
	std::ifstream dbconf((Wt::WApplication::appRoot()
			      + "HangmanDb.info").c_str());
	dbconf >> retval;
	return retval;
}

std::string HangmanDb::DbPass()
{
	std::string retval;
	std::ifstream dbconf((Wt::WApplication::appRoot()
			      + "HangmanDb.info").c_str());
	dbconf >> retval; // username
	dbconf >> retval; // password
	return retval;
}

// this function returns false if user existed, true if user inserted
// It guarantees atomic userExists() checking and adding it if the user
// did not yet exits.
bool HangmanDb::addUser(const std::wstring &user, const std::wstring &password)
{
   try {
      Connection con("hangman", "localhost", DbUser().c_str(), DbPass().c_str());
      Query q = con.query();
      q << "insert into users "
	<< "set user='" << Wt::toUTF8(user) << "', pass=MD5('"
	<< Wt::toUTF8(password) << "'), numgames=0, score=0, lastseen=now()";
      q.store();
      return true;
   } catch(Exception &e) {
      std::cerr << "Database exception!\n";
      std::cerr << e.what() << std::endl;
      return false;
   }
}

bool HangmanDb::validLogin(const std::wstring &user, const std::wstring &pass)
{
   try {
      Connection con("hangman", "localhost", DbUser().c_str(), DbPass().c_str());
      Query q = con.query();
      q << "select user,pass from users where "
	<< "user='" << Wt::toUTF8(user)
	<< "' and pass=MD5('" << Wt::toUTF8(pass) << "')";
      StoreQueryResult res = q.store();
      return res.size() > 0;
   } catch(Exception &e) {
      std::cerr << "Database exception!\n";
      std::cerr << e.what() << std::endl;
      return false;
   }
}

void HangmanDb::addToScore(const std::wstring &user, int delta)
{
   try {
      Connection con("hangman", "localhost", DbUser().c_str(), DbPass().c_str());
      Query q = con.query();
      q << "update users set score=(score+" << delta << "), "
	<< "numgames=(numgames+1), lastseen=now() "
	<< "where user='" << Wt::toUTF8(user) << "'";
      StoreQueryResult res = q.store();
   } catch(Exception &e) {
      std::cerr << "Database exception!\n";
      std::cerr << e.what() << std::endl;
   }
}

std::vector<HangmanDb::Score> HangmanDb::getHighScores(int top)
{
   std::vector<HangmanDb::Score> retval;
   try {
      Connection con("hangman", "localhost", DbUser().c_str(), DbPass().c_str());
      Query q = con.query();
      q << "select user, numgames, score, lastseen from users "
	<< "order by score desc "
	<< "limit " << top;
      StoreQueryResult res = q.store();

      for(unsigned int i = 0; i < res.size(); ++i) {
	 struct Score s;
	 s.number = i + 1;
	 s.user = Wt::fromUTF8((std::string)res.at(i)["user"]);
	 s.numgames = res.at(i)["numgames"];
	 s.score = res.at(i)["score"];
	 s.lastseen = Wt::fromUTF8((std::string)res.at(i)["lastseen"]);
	 retval.push_back(s);
      }
   } catch(Exception &e) {
      std::cerr << "Database exception!\n";
      std::cerr << e.what() << std::endl;
   }
   return retval;
}

HangmanDb::Score HangmanDb::getUserPosition(const std::wstring &user)
{
   try {
      Connection con("hangman", "localhost", DbUser().c_str(), DbPass().c_str());
      Query q = con.query();
      q << "select user, numgames, score, lastseen from users "
	<< "order by score desc";
      StoreQueryResult res = q.store();

      // There MUST be a better way to do this...
      for(unsigned int i = 0; i < res.size(); ++i) {
	 if(Wt::fromUTF8((std::string)res.at(i)["user"]) == user) {
	    struct Score s;
	    s.number = i + 1;
	    s.user = Wt::fromUTF8((std::string)res.at(i)["user"]);
	    s.numgames = res.at(i)["numgames"];
	    s.score = res.at(i)["score"];
	    s.lastseen = Wt::fromUTF8((std::string)res.at(i)["lastseen"]);
	    return s;
	 }
      }
   } catch(Exception &e) {
      std::cerr << "Database exception!\n";
      std::cerr << e.what() << std::endl;
   }
   Score s;
   s.number=0;
   s.user=L"DBase error";
   s.numgames = s.score = 0;
   return s;
}
