/*
 * Copyright (C) 2017 Emweb bv, Herent, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include <boost/test/unit_test.hpp>

#include "Wt/Test/WTestEnvironment.h"

#include "Wt/WEvent.h"
#include "Wt/WPointF.h"
#include "Wt/WSslInfo.h"
#include "web/CgiParser.h"
#include "web/WebRequest.h"

namespace {
  class MockRequest : public Wt::WebRequest {
  public:
    bool supportsTransferWebSocketResourceSocket() override
    {
      return false;
    }

    virtual void flush(WT_MAYBE_UNUSED ResponseState state, WT_MAYBE_UNUSED const WriteCallback& callback) override
    { }

    virtual std::istream &in() override
    {
      return in_;
    }

    virtual std::ostream &out() override
    {
      return out_;
    }

    virtual std::ostream &err() override
    {
      return err_;
    }

    virtual void setRedirect(WT_MAYBE_UNUSED const std::string& url) override
    { }

    virtual void setStatus(WT_MAYBE_UNUSED int status) override
    { }

    int status() override
    {
      return 0;
    }

    virtual void setContentType(const std::string &value) override
    {
      contentType_ = value;
    }

    virtual const char *contentType() const override
    {
      return contentType_.c_str();
    }

    virtual void setContentLength(int64_t length) override
    {
      contentLength_ = length;
    }

    virtual int64_t contentLength() const override
    {
      return contentLength_;
    }

    virtual void addHeader(WT_MAYBE_UNUSED const std::string& name, WT_MAYBE_UNUSED const std::string& value) override
    { }

    void insertHeader(WT_MAYBE_UNUSED const std::string &name, WT_MAYBE_UNUSED const std::string &value) override
    { }

    virtual const char *envValue(WT_MAYBE_UNUSED const char *name) const override
    {
      return nullptr;
    }

    virtual const std::string &serverName() const override
    {
      return serverName_;
    }

    virtual const std::string &serverPort() const override
    {
      return serverPort_;
    }

    virtual const std::string &scriptName() const override
    {
      return scriptName_;
    }

    virtual const char *requestMethod() const override
    {
      return requestMethod_.c_str();
    }

    virtual const std::string &queryString() const override
    {
      return queryString_;
    }

    virtual const std::string &pathInfo() const override
    {
      return pathInfo_;
    }

    virtual const std::string &remoteAddr() const override
    {
      return remoteAddr_;
    }

    virtual const char *urlScheme() const override
    {
      return urlScheme_.c_str();
    }

    virtual const char *headerValue(WT_MAYBE_UNUSED const char* name) const override
    {
      return nullptr;
    }

    virtual std::vector<Wt::Http::Message::Header> headers() const override
    {
      return std::vector<Wt::Http::Message::Header>{};
    }

    virtual std::unique_ptr<Wt::WSslInfo> sslInfo(const Wt::Configuration &) const override
    {
      return nullptr;
    }

    std::string contentType_;
    int64_t contentLength_;
    std::stringstream in_;
    std::stringstream out_;
    std::stringstream err_;
    std::string serverName_;
    std::string serverPort_;
    std::string scriptName_;
    std::string requestMethod_;
    std::string queryString_;
    std::string pathInfo_;
    std::string remoteAddr_;
    std::string urlScheme_;
  };
}

BOOST_AUTO_TEST_CASE( CgiParser_multipart_parsing_quoted_values )
{
  MockRequest request;

  std::string content = "--ExampleBoundaryString\r\n"
                        "Content-Disposition: form-data; name=\"description\"\r\n"
                        "\r\n"
                        "Description input value\r\n"
                        "--ExampleBoundaryString\r\n"
                        "Content-Disposition: form-data; name=\"myFile\"; filename=\"foo.txt\"\r\n"
                        "Content-Type: text/plain\r\n"
                        "\r\n"
                        "This is some text content\r\n"
                        "--ExampleBoundaryString--";

  request.setContentLength(content.length());
  request.setContentType("multipart/form-data; boundary=\"ExampleBoundaryString\"");
  request.requestMethod_ = "POST";
  request.in_.str(content);

  Wt::CgiParser parser{1024, 1024};
  parser.parse(request, Wt::CgiParser::ReadDefault);

  auto files = request.uploadedFiles();
  auto params = request.getParameterMap();

  BOOST_TEST(files.size() == 1);
  BOOST_TEST(params.size() == 1);

  BOOST_REQUIRE(files.find("myFile") != files.end());
  BOOST_TEST(files.find("myFile")->second.clientFileName() == "foo.txt");
  BOOST_TEST(files.find("myFile")->second.contentType() == "text/plain");

  BOOST_REQUIRE(params.find("description") != params.end());
  BOOST_REQUIRE(params.find("description")->second.size() == 1);
  BOOST_TEST(params.find("description")->second[0] == "Description input value");
}

BOOST_AUTO_TEST_CASE( CgiParser_multipart_parsing_multiple_values_same_name )
{
  MockRequest request;

  std::string content = "--B\r\n"
                        "Content-Disposition: form-data; name=\"tags\"\r\n"
                        "\r\n"
                        "one\r\n"
                        "--B\r\n"
                        "Content-Disposition: form-data; name=\"tags\"\r\n"
                        "\r\n"
                        "two\r\n"
                        "--B--";

  request.setContentLength(content.length());
  request.setContentType("multipart/form-data; boundary=B");
  request.requestMethod_ = "POST";
  request.in_.str(content);

  Wt::CgiParser parser{1024, 1024};
  parser.parse(request, Wt::CgiParser::ReadDefault);

  auto params = request.getParameterMap();

  BOOST_TEST(params.size() == 1);

  BOOST_REQUIRE(params.find("tags") != params.end());
  BOOST_REQUIRE(params.find("tags")->second.size() == 2);
  BOOST_TEST(params.find("tags")->second[0] == "one");
  BOOST_TEST(params.find("tags")->second[1] == "two");
}

BOOST_AUTO_TEST_CASE( CgiParser_multipart_parsing_file_without_content_type )
{
  MockRequest request;

  std::string content = "--B\r\n"
                        "Content-Disposition: form-data; name=\"myFile\"; filename=\"foo.txt\"\r\n"
                        "\r\n"
                        "This is file content\r\n"
                        "--B--";

  request.setContentLength(content.length());
  request.setContentType("multipart/form-data; boundary=B");
  request.requestMethod_ = "POST";
  request.in_.str(content);

  Wt::CgiParser parser{1024, 1024};
  parser.parse(request, Wt::CgiParser::ReadDefault);

  auto files = request.uploadedFiles();

  BOOST_TEST(files.size() == 1);
  BOOST_REQUIRE(files.find("myFile") != files.end());
  BOOST_TEST(files.find("myFile")->second.clientFileName() == "foo.txt");
  BOOST_TEST(files.find("myFile")->second.contentType().empty());
}

BOOST_AUTO_TEST_CASE( CgiParser_multipart_parsing_unquoted_values )
{
  MockRequest request;

  std::string content = "--ExampleBoundaryString\r\n"
                        "Content-Disposition: form-data; name=description\r\n"
                        "\r\n"
                        "Description input value\r\n"
                        "--ExampleBoundaryString\r\n"
                        "Content-Disposition: form-data; name=myFile; filename=foo.txt\r\n"
                        "Content-Type: text/plain\r\n"
                        "\r\n"
                        "This is some text content\r\n"
                        "--ExampleBoundaryString--";

  request.setContentLength(content.length());
  request.setContentType("multipart/form-data; boundary=ExampleBoundaryString");
  request.requestMethod_ = "POST";
  request.in_.str(content);

  Wt::CgiParser parser{1024, 1024};
  parser.parse(request, Wt::CgiParser::ReadDefault);

  auto files = request.uploadedFiles();
  auto params = request.getParameterMap();

  BOOST_TEST(files.size() == 1);
  BOOST_TEST(params.size() == 1);

  BOOST_REQUIRE(files.find("myFile") != files.end());
  BOOST_TEST(files.find("myFile")->second.clientFileName() == "foo.txt");
  BOOST_TEST(files.find("myFile")->second.contentType() == "text/plain");

  BOOST_REQUIRE(params.find("description") != params.end());
  BOOST_REQUIRE(params.find("description")->second.size() == 1);
  BOOST_TEST(params.find("description")->second[0] == "Description input value");
}

BOOST_AUTO_TEST_CASE( CgiParser_multipart_parsing_boundary_too_long_unquoted )
{
  MockRequest request;

  std::string content = "--ThisBoundaryStringIsWayTooLongTheMaximumAllowedLengthIsOnly70Characters\r\n"
                        "Content-Disposition: form-data; name=\"description\"\r\n"
                        "\r\n"
                        "Description input value\r\n"
                        "--ThisBoundaryStringIsWayTooLongTheMaximumAllowedLengthIsOnly70Characters\r\n"
                        "Content-Disposition: form-data; filename=\"foo.txt\"; name=\"myFile\"\r\n"
                        "Content-Type: text/plain\r\n"
                        "\r\n"
                        "This is some text content\r\n"
                        "--ThisBoundaryStringIsWayTooLongTheMaximumAllowedLengthIsOnly70Characters--";

  request.setContentLength(content.length());
  request.setContentType("multipart/form-data; boundary=ThisBoundaryStringIsWayTooLongTheMaximumAllowedLengthIsOnly70Characters");
  request.requestMethod_ = "POST";
  request.in_.str(content);

  Wt::CgiParser parser{1024, 1024};

  BOOST_CHECK_THROW(parser.parse(request, Wt::CgiParser::ReadDefault), Wt::WException);
}

BOOST_AUTO_TEST_CASE( CgiParser_multipart_parsing_boundary_too_long_quoted )
{
  MockRequest request;

  std::string content = "--ThisBoundaryStringIsWayTooLongTheMaximumAllowedLengthIsOnly70Characters\r\n"
                        "Content-Disposition: form-data; name=\"description\"\r\n"
                        "\r\n"
                        "Description input value\r\n"
                        "--ThisBoundaryStringIsWayTooLongTheMaximumAllowedLengthIsOnly70Characters\r\n"
                        "Content-Disposition: form-data; filename=\"foo.txt\"; name=\"myFile\"\r\n"
                        "Content-Type: text/plain\r\n"
                        "\r\n"
                        "This is some text content\r\n"
                        "--ThisBoundaryStringIsWayTooLongTheMaximumAllowedLengthIsOnly70Characters--";

  request.setContentLength(content.length());
  request.setContentType("multipart/form-data; boundary=\"ThisBoundaryStringIsWayTooLongTheMaximumAllowedLengthIsOnly70Characters\"");
  request.requestMethod_ = "POST";
  request.in_.str(content);

  Wt::CgiParser parser{1024, 1024};

  BOOST_CHECK_THROW(parser.parse(request, Wt::CgiParser::ReadDefault), Wt::WException);
}

BOOST_AUTO_TEST_CASE( CgiParser_multipart_parsing_max_size_boundary_unquoted )
{
  MockRequest request;

  std::string content = "--ThisBoundaryStringIsExactlyTheMaximumAllowedLengthWithOnly70Characters\r\n"
                        "Content-Disposition: form-data; name=\"description\"\r\n"
                        "\r\n"
                        "Description input value\r\n"
                        "--ThisBoundaryStringIsExactlyTheMaximumAllowedLengthWithOnly70Characters\r\n"
                        "Content-Disposition: form-data; filename=\"foo.txt\"; name=\"myFile\"\r\n"
                        "Content-Type: text/plain\r\n"
                        "\r\n"
                        "This is some text content\r\n"
                        "--ThisBoundaryStringIsExactlyTheMaximumAllowedLengthWithOnly70Characters--";

  request.setContentLength(content.length());
  request.setContentType("multipart/form-data; boundary=ThisBoundaryStringIsExactlyTheMaximumAllowedLengthWithOnly70Characters");
  request.requestMethod_ = "POST";
  request.in_.str(content);

  Wt::CgiParser parser{1024, 1024};
  parser.parse(request, Wt::CgiParser::ReadDefault);

  auto files = request.uploadedFiles();
  auto params = request.getParameterMap();

  BOOST_TEST(files.size() == 1);
  BOOST_TEST(params.size() == 1);

  BOOST_REQUIRE(files.find("myFile") != files.end());
  BOOST_TEST(files.find("myFile")->second.clientFileName() == "foo.txt");
  BOOST_TEST(files.find("myFile")->second.contentType() == "text/plain");

  BOOST_REQUIRE(params.find("description") != params.end());
  BOOST_REQUIRE(params.find("description")->second.size() == 1);
  BOOST_TEST(params.find("description")->second[0] == "Description input value");
}

BOOST_AUTO_TEST_CASE( CgiParser_multipart_parsing_max_size_boundary_quoted )
{
  MockRequest request;

  std::string content = "--ThisBoundaryStringIsExactlyTheMaximumAllowedLengthWithOnly70Characters\r\n"
                        "Content-Disposition: form-data; name=\"description\"\r\n"
                        "\r\n"
                        "Description input value\r\n"
                        "--ThisBoundaryStringIsExactlyTheMaximumAllowedLengthWithOnly70Characters\r\n"
                        "Content-Disposition: form-data; filename=\"foo.txt\"; name=\"myFile\"\r\n"
                        "Content-Type: text/plain\r\n"
                        "\r\n"
                        "This is some text content\r\n"
                        "--ThisBoundaryStringIsExactlyTheMaximumAllowedLengthWithOnly70Characters--";

  request.setContentLength(content.length());
  request.setContentType("multipart/form-data; boundary=\"ThisBoundaryStringIsExactlyTheMaximumAllowedLengthWithOnly70Characters\"");
  request.requestMethod_ = "POST";
  request.in_.str(content);

  Wt::CgiParser parser{1024, 1024};
  parser.parse(request, Wt::CgiParser::ReadDefault);

  auto files = request.uploadedFiles();
  auto params = request.getParameterMap();

  BOOST_TEST(files.size() == 1);
  BOOST_TEST(params.size() == 1);

  BOOST_REQUIRE(files.find("myFile") != files.end());
  BOOST_TEST(files.find("myFile")->second.clientFileName() == "foo.txt");
  BOOST_TEST(files.find("myFile")->second.contentType() == "text/plain");

  BOOST_REQUIRE(params.find("description") != params.end());
  BOOST_REQUIRE(params.find("description")->second.size() == 1);
  BOOST_TEST(params.find("description")->second[0] == "Description input value");
}

BOOST_AUTO_TEST_CASE( CgiParser_multipart_parsing_boundary_empty_unquoted )
{
  MockRequest request;

  std::string content = "--\r\n"
                        "Content-Disposition: form-data; name=\"description\"\r\n"
                        "\r\n"
                        "Description input value\r\n"
                        "--\r\n"
                        "Content-Disposition: form-data; filename=\"foo.txt\"; name=\"myFile\"\r\n"
                        "Content-Type: text/plain\r\n"
                        "\r\n"
                        "This is some text content\r\n"
                        "----";

  request.setContentLength(content.length());
  request.setContentType("multipart/form-data; boundary=");
  request.requestMethod_ = "POST";
  request.in_.str(content);

  Wt::CgiParser parser{1024, 1024};

  BOOST_CHECK_THROW(parser.parse(request, Wt::CgiParser::ReadDefault), Wt::WException);
}

BOOST_AUTO_TEST_CASE( CgiParser_multipart_parsing_boundary_empty_quoted )
{
  MockRequest request;

  std::string content = "--\r\n"
                        "Content-Disposition: form-data; name=\"description\"\r\n"
                        "\r\n"
                        "Description input value\r\n"
                        "--\r\n"
                        "Content-Disposition: form-data; filename=\"foo.txt\"; name=\"myFile\"\r\n"
                        "Content-Type: text/plain\r\n"
                        "\r\n"
                        "This is some text content\r\n"
                        "----";

  request.setContentLength(content.length());
  request.setContentType("multipart/form-data; boundary=\"\"");
  request.requestMethod_ = "POST";
  request.in_.str(content);

  Wt::CgiParser parser{1024, 1024};

  BOOST_CHECK_THROW(parser.parse(request, Wt::CgiParser::ReadDefault), Wt::WException);
}

BOOST_AUTO_TEST_CASE( CgiParser_multipart_parsing_boundary_missing_end_quote )
{
  MockRequest request;

  std::string content = "--aaa\r\n"
                        "Content-Disposition: form-data; name=\"description\"\r\n"
                        "\r\n"
                        "Description input value\r\n"
                        "--aaa\r\n"
                        "Content-Disposition: form-data; filename=\"foo.txt\"; name=\"myFile\"\r\n"
                        "Content-Type: text/plain\r\n"
                        "\r\n"
                        "This is some text content\r\n"
                        "--aaa--";

  request.setContentLength(content.length());
  request.setContentType("multipart/form-data; \"boundary=aaa");
  request.requestMethod_ = "POST";
  request.in_.str(content);

  Wt::CgiParser parser{1024, 1024};

  BOOST_CHECK_THROW(parser.parse(request, Wt::CgiParser::ReadDefault), Wt::WException);
}

BOOST_AUTO_TEST_CASE( CgiParser_multipart_parsing_parameter_missing_end_quote )
{
  MockRequest request;

  std::string content = "--aaa\r\n"
                        "Content-Disposition: form-data; name=\"description\r\n"
                        "\r\n"
                        "Description input value\r\n"
                        "--aaa\r\n"
                        "Content-Disposition: form-data; filename=\"foo.txt\"; name=\"myFile\"\r\n"
                        "Content-Type: text/plain\r\n"
                        "\r\n"
                        "This is some text content\r\n"
                        "--aaa--";

  request.setContentLength(content.length());
  request.setContentType("multipart/form-data; boundary=aaa");
  request.requestMethod_ = "POST";
  request.in_.str(content);

  Wt::CgiParser parser{1024, 1024};
  parser.parse(request, Wt::CgiParser::ReadDefault);

  auto files = request.uploadedFiles();
  auto params = request.getParameterMap();

  BOOST_TEST(files.size() == 1);
  BOOST_TEST(params.size() == 0);

  BOOST_REQUIRE(files.find("myFile") != files.end());
  BOOST_TEST(files.find("myFile")->second.clientFileName() == "foo.txt");
  BOOST_TEST(files.find("myFile")->second.contentType() == "text/plain");
}

BOOST_AUTO_TEST_CASE( CgiParser_multipart_parsing_parameter_in_value )
{
  MockRequest request;

  std::string content = "--ExampleBoundaryString\r\n"
                        "Content-Disposition: form-data; name=\"description\"\r\n"
                        "\r\n"
                        "Description input value\r\n"
                        "--ExampleBoundaryString\r\n"
                        "Content-Disposition: form-data; name=\" filename=wrong\"; filename=\"foo.txt\"\r\n"
                        "Content-Type: text/plain\r\n"
                        "\r\n"
                        "This is some text content\r\n"
                        "--ExampleBoundaryString--";

  request.setContentLength(content.length());
  request.setContentType("multipart/form-data; boundary=\"ExampleBoundaryString\"");
  request.requestMethod_ = "POST";
  request.in_.str(content);

  Wt::CgiParser parser{1024, 1024};
  parser.parse(request, Wt::CgiParser::ReadDefault);

  auto files = request.uploadedFiles();
  auto params = request.getParameterMap();

  BOOST_TEST(files.size() == 1);
  BOOST_TEST(params.size() == 1);

  BOOST_REQUIRE(files.find(" filename=wrong") != files.end());
  BOOST_TEST(files.find(" filename=wrong")->second.clientFileName() == "foo.txt");
  BOOST_TEST(files.find(" filename=wrong")->second.contentType() == "text/plain");

  BOOST_REQUIRE(params.find("description") != params.end());
  BOOST_REQUIRE(params.find("description")->second.size() == 1);
  BOOST_TEST(params.find("description")->second[0] == "Description input value");
}

BOOST_AUTO_TEST_CASE( CgiParser_multipart_header_too_long )
{
  MockRequest request;

  std::string content = "--AaB0\r\n";
  content +=            "Content-Disposition: form-data; name=\"a\"\r\n";

  for (int i = 0; i < 314; ++i) {
    content +=          "Content-Type: text/plain\r\n";
  }

  content +=            "\r\n";
  content +=            "skddfjhKSQF\r\n";
  content +=            "--AaB0--";

  request.setContentLength(content.length());
  request.setContentType("multipart/form-data; boundary=\"AaB0\"");
  request.requestMethod_ = "POST";
  request.in_.str(content);

  Wt::CgiParser parser{65536, 65536};
  BOOST_CHECK_THROW(parser.parse(request, Wt::CgiParser::ReadDefault), Wt::WException);
}

BOOST_AUTO_TEST_CASE( CgiParser_multipart_header_almost_too_long )
{
  MockRequest request;

  std::string content = "--AaB0\r\n";
  content +=            "Content-Disposition: form-data; name=\"a\"\r\n";

  for (int i = 0; i < 313; ++i) {
    content +=          "Content-Type: text/plain\r\n";
  }

  content +=            "\r\n";
  content +=            "skddfjhKSQF\r\n";
  content +=            "--AaB0--";

  request.setContentLength(content.length());
  request.setContentType("multipart/form-data; boundary=\"AaB0\"");
  request.requestMethod_ = "POST";
  request.in_.str(content);

  Wt::CgiParser parser{65536, 65536};
  parser.parse(request, Wt::CgiParser::ReadDefault);

  auto files = request.uploadedFiles();
  auto params = request.getParameterMap();

  BOOST_TEST(files.size() == 0);
  BOOST_TEST(params.size() == 1);

  BOOST_REQUIRE(params.find("a") != params.end());
  BOOST_REQUIRE(params.find("a")->second.size() == 1);
  BOOST_TEST(params.find("a")->second[0] == "skddfjhKSQF");
}

BOOST_AUTO_TEST_CASE( CgiParser_multipart_name_too_long )
{
  MockRequest request;

  std::string tooLongName;
  for (int i = 0; i < 256; ++i) {
    tooLongName += "a";
  }
  std::string almostTooLongName = tooLongName.substr(0, 255);

  std::string content = "--AaB0\r\n"
                        "Content-Disposition: form-data; name=\"" + tooLongName + "\"\r\n"
                        "Content-Type: text/plain\r\n"
                        "\r\n"
                        "skddfjhKSQF\r\n"
                        "--AaB0\r\n"
                        "Content-Disposition: form-data; name=\"" + almostTooLongName + "\"\r\n"
                        "Content-Type: text/plain\r\n"
                        "\r\n"
                        "skddfjhKSzedzefzaeazQF\r\n"
                        "--AaB0--";

  request.setContentLength(content.length());
  request.setContentType("multipart/form-data; boundary=\"AaB0\"");
  request.requestMethod_ = "POST";
  request.in_.str(content);

  Wt::CgiParser parser{65536, 65536};
  parser.parse(request, Wt::CgiParser::ReadDefault);

  auto files = request.uploadedFiles();
  auto params = request.getParameterMap();

  // The first parameter is ignored because the name is too long.
  BOOST_TEST(files.size() == 0);
  BOOST_TEST(params.size() == 1);

  BOOST_REQUIRE(params.find(almostTooLongName) != params.end());
  BOOST_REQUIRE(params.find(almostTooLongName)->second.size() == 1);
  BOOST_TEST(params.find(almostTooLongName)->second[0] == "skddfjhKSzedzefzaeazQF");
}

BOOST_AUTO_TEST_CASE( CgiParser_multipart_filename_too_long )
{
  MockRequest request;

  std::string tooLongFilename;
  std::string almostTooLongFilename;
  for (int i = 0; i < 1020; ++i) {
    tooLongFilename += "a";
    almostTooLongFilename += "b";
  }
  tooLongFilename += ".ttxt";
  almostTooLongFilename += ".txt";

  std::string content = "--AaB0\r\n"
                        "Content-Disposition: form-data; name=\"a\" filename=\"" + tooLongFilename + "\"\r\n"
                        "Content-Type: text/plain\r\n"
                        "\r\n"
                        "skddfjhKSQF\r\n"
                        "--AaB0\r\n"
                        "Content-Disposition: form-data; name=\"b\"; filename=\"" + almostTooLongFilename + "\"\r\n"
                        "Content-Type: text/plain\r\n"
                        "\r\n"
                        "skddfjhKSzedzefzaeazQF\r\n"
                        "--AaB0--";

  request.setContentLength(content.length());
  request.setContentType("multipart/form-data; boundary=\"AaB0\"");
  request.requestMethod_ = "POST";
  request.in_.str(content);

  Wt::CgiParser parser{65536, 65536};
  parser.parse(request, Wt::CgiParser::ReadDefault);

  auto files = request.uploadedFiles();
  auto params = request.getParameterMap();

  // The first filename is truncated because it is too long.
  BOOST_TEST(files.size() == 2);
  BOOST_TEST(params.size() == 0);

  BOOST_REQUIRE(files.find("a") != files.end());
  BOOST_TEST(files.find("a")->second.clientFileName() == tooLongFilename.substr(1));
  BOOST_TEST(files.find("a")->second.contentType() == "text/plain");

  BOOST_REQUIRE(files.find("b") != files.end());
  BOOST_TEST(files.find("b")->second.clientFileName() == almostTooLongFilename);
  BOOST_TEST(files.find("b")->second.contentType() == "text/plain");
}

BOOST_AUTO_TEST_CASE( CgiParser_multipart_parameter_too_long )
{
  MockRequest request;

  // make a string of 32769 characters (2^15 + 1)
  std::string parameterValue = "a";
  for (int i = 0; i < 15; ++i) {
    parameterValue += parameterValue;
  }
  parameterValue += "a";

  std::string content = "--AaB0\r\n"
                        "Content-Disposition: form-data; name=\"a\"\r\n"
                        "Content-Type: text/plain\r\n"
                        "\r\n"
                        + parameterValue + "\r\n"
                        "--AaB0--";

  request.setContentLength(content.length());
  request.setContentType("multipart/form-data; boundary=\"AaB0\"");
  request.requestMethod_ = "POST";
  request.in_.str(content);

  Wt::CgiParser parser{65536, 65536};
  BOOST_CHECK_THROW(parser.parse(request, Wt::CgiParser::ReadDefault), Wt::WException);
}

BOOST_AUTO_TEST_CASE( CgiParser_multipart_parameter_almost_too_long )
{
  MockRequest request;

  // make a string of 32768 characters (2^15)
  std::string parameterValue = "a";
  for (int i = 0; i < 15; ++i) {
    parameterValue += parameterValue;
  }

  std::string content = "--AaB0\r\n"
                        "Content-Disposition: form-data; name=\"b\"\r\n"
                        "Content-Type: text/plain\r\n"
                        "\r\n"
                        + parameterValue + "\r\n"
                        "--AaB0--";

  request.setContentLength(content.length());
  request.setContentType("multipart/form-data; boundary=\"AaB0\"");
  request.requestMethod_ = "POST";
  request.in_.str(content);

  Wt::CgiParser parser{65536, 65536};
  parser.parse(request, Wt::CgiParser::ReadDefault);

  auto files = request.uploadedFiles();
  auto params = request.getParameterMap();

  BOOST_TEST(files.size() == 0);
  BOOST_TEST(params.size() == 1);

  BOOST_REQUIRE(params.find("b") != params.end());
  BOOST_REQUIRE(params.find("b")->second.size() == 1);
  BOOST_TEST(params.find("b")->second[0] == parameterValue);
}
