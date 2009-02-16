#ifndef DB_H_
#define DB_H_

#include <string>
#include <vector>

class HangmanDb
{
public:
  // this function returns false if user existed, true if user inserted
  // It guarantees atomic userExists() checking and adding it if the user
  // did not yet exits.
  static bool addUser(const std::wstring &user, const std::wstring &password);

  // This function returns true when the credentials are found in the
  // database, otherwise false
  static bool validLogin(const std::wstring &user, const std::wstring &pass);

  // Increments the number of games played, and adds delta to the score
  static void addToScore(const std::wstring &user, int delta);

  struct Score {
    int number; // position of the user
    std::wstring user;
    int numgames;
    int score;
    std::wstring lastseen; // Last seen, in GMT
  };

  // Returns the top n highest scoring users
  static std::vector<Score> getHighScores(int top);

  // Returns the score structure for the given user
  static Score getUserPosition(const std::wstring &user);

private:
  static std::string DbUser();
  static std::string DbPass();
};

#endif
