/*
 * Copyright (C) 2011 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <iostream>
#include <fstream>
#include <boost/test/unit_test.hpp>

#include <Wt/Mail/Client.h>
#include <Wt/Mail/Message.h>
#include <Wt/WLocalDateTime.h>

using namespace Wt;
using namespace Wt::Mail;

BOOST_AUTO_TEST_CASE( mail_test1 )
{
  Message m;
  m.setFrom(Mailbox("bas@kode.be", "Bas Deforche"));
  m.addRecipient(RecipientType::To, Mailbox("koen@emweb.be", "Koen Deforche"));
  m.addRecipient(RecipientType::Bcc,
		 Mailbox("koen.deforche@gmail.com",
			 WString::fromUTF8("Koen Deforche")));
  m.setSubject(WString::fromUTF8("Hey there, \xe2\x82\xac !"));
  m.setBody(WString::fromUTF8
	    ("Body here \xe2\x82\xac\n"
	     "We have been working hard\n"
	     ".beware this\n"
	     "And long lines should be properly split using a soft line end,"
	     "let's see how that turns out.\n"
	     "But a space before a new line needs some special handling. \n"
	     "Are we good?"));
  m.addHtmlBody(WString::fromUTF8
		("<div>"
		 "<h1>HTML body here</h1>"
		 "Long lines should be properly split using a soft line "
		 "end, let's see how that turns out.<br>"
		 "This is necessary to make lots of \xe2\x82\xac! "
		 "Please visit <a href=\"http://www.emweb.be\">Emweb</a>."
		 "</div>"));

#if 0
  Client c;
  c.connect("localhost");
  c.send(m);
#else
  m.write(std::cout);
#endif
}

BOOST_AUTO_TEST_CASE( mail_test2 )
{
  Message m;
  m.setFrom(Mailbox("bas@kode.be", "Bas Deforche"));
  m.setDate(WLocalDateTime::currentServerDateTime());
  m.addRecipient(RecipientType::To, Mailbox("koen@emweb.be", "Koen Deforche"));
  m.addRecipient(RecipientType::Bcc,
		 Mailbox("koen.deforche@gmail.com",
			 WString::fromUTF8("Koen Deforche")));
  m.setSubject(WString::fromUTF8("Hey there, \xe2\x82\xac !"));
  m.setBody(WString::fromUTF8
	    ("Body here \xe2\x82\xac\n"
	     "We have been working hard\n"
	     ".beware this\n"
	     "And long lines should be properly split using a soft line end,"
	     "let's see how that turns out."));
  m.addHtmlBody(WString::fromUTF8
		("<div>"
		 "<h1>HTML body here</h1>"
		 "Long lines should be properly split using a soft line "
		 "end, let's see how that turns out.<br>"
		 "This is necessary to make lots of \xe2\x82\xac! "
		 "Please visit <a href=\"http://www.emweb.be\">Emweb</a>."
		 "</div>"));
  std::ifstream pdf("test.pdf");
  m.addAttachment("application/pdf", "hello.pdf", &pdf);

#if 0
  Client c;
  c.connect("localhost");
  c.send(m);
#else
  m.write(std::cout);
#endif
}
