/*
 * Copyright (C) 2008 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>

#include <Wt/WContainerWidget.h>
#include <Wt/WStringUtil.h>

#include "AddresseeEdit.h"
#include "Label.h"

AddresseeEdit::AddresseeEdit(const WString& label, WContainerWidget *labelParent)
  : WTextArea()
{
  label_ = labelParent->addWidget(cpp14::make_unique<Label>(label, labelParent));

  setRows(3); setColumns(55);
  resize(WLength(99, LengthUnit::Percentage), WLength::Auto);

  setInline(false); // for IE to position the suggestions well
}

void AddresseeEdit::setAddressees(const std::vector<Contact>& contacts)
{
  std::u32string text;

  for (unsigned i = 0; i < contacts.size(); ++i) {
    if (i != 0)
      text += U", ";
    text += contacts[i].formatted();
  }

  setText(text);
}

bool AddresseeEdit::parse(std::vector<Contact>& contacts) const
{
  typedef boost::tokenizer<boost::escaped_list_separator<char32_t>,
                           std::wstring::const_iterator, std::wstring>
    CsvTokenizer;

  std::wstring t = text();
  CsvTokenizer tok(t);
  
 for (CsvTokenizer::iterator i = tok.begin(); i != tok.end(); ++i) {
    std::wstring addressee = *i;

    boost::trim(addressee);
    std::wstring::size_type pos = addressee.find_last_of(' ');
    if (pos != std::string::npos) {
      std::wstring email = addressee.substr(pos + 1);
      std::wstring name = addressee.substr(0, pos);

      boost::trim(email);
      boost::trim(name);
      if (email[0] == '<')
	email = email.substr(1);
      if (email[email.length() - 1] == '>')
	email = email.substr(0, email.length() - 1);

      if (!email.empty())
	contacts.push_back(Contact(Wt::toUTF32(name), Wt::toUTF32(email)));
    } else
      if (!addressee.empty())
        contacts.push_back(Contact(U"", Wt::toUTF32(addressee)));
  }
  return true;
}

std::vector<Contact> AddresseeEdit::addressees() const
{
  std::vector<Contact> result;
  parse(result);

  return result;
}

void AddresseeEdit::setHidden(bool hidden, const WAnimation& animation)
{
  WTextArea::setHidden(hidden, animation);
  label_->setHidden(hidden, animation);
}
