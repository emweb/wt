#ifndef USERDETAILSMODEL_H
#define USERDETAILSMODEL_H

#include <Wt/WFormModel.h>

class Session;

class UserDetailsModel : public Wt::WFormModel
{
public:
  static const Field NameField;

  UserDetailsModel(Session& session);

  void save(const Wt::Auth::User& user);

private:
  Session& session_;
};

#endif // USERDETAILSMODEL_H
