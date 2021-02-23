#include "TextEditFormDelegate.h"

#include <Wt/WTextEdit.h>

TextEditFormDelegate::TextEditFormDelegate()
{
}

std::unique_ptr<Wt::WWidget> TextEditFormDelegate::createFormWidget()
{
  return std::make_unique<Wt::WTextEdit>();
}
