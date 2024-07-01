#include "myuser.h"

DBO_INSTANTIATE_TEMPLATES(MyUser)

MyUser::MyUser(const std::string& name)
  : name_(name)
{
}
